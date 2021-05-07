#include <stdio.h>
#include "mg_pool.h"
#include <stddef.h>

long mem_count = 0;		/*detect mem leak*/


/*
创建内存池
b_pool means base pool;
如果b_pool为NULL，则创建根内存池，其父池为自身。
b_pool为有效值时，则创建b_pool的子池。
当父池被释放时，子池也会被释放。
内存池中分配的储存单元，都会额外分配一个mg_pool_t类型的头部，认为分配后不可读，但是越界访问可能会改变他的值，会引起意外错误。
对于pool 本身也有一个头部。
其头部的type说明内存中的数据为MG_MEM_NOR 通用数据，MG_MEM_POOL 一个子池。
头部的size为数据区大小。
如果储存单元数据区为pool类型，它的type和size将为无意义的值，也不应该访问这些值。
*/
mg_pool_t * mg_pool_create(mg_pool_t	* b_pool)
{
	mg_pool_t	*pool = NULL;							/*pool struct*/

	if(NULL == b_pool)									/*create root pool*/
	{															/*root looks like 2 mem heads*/
		pool = (mg_pool_t *)malloc(sizeof(mg_pool_t)*2);
		if(NULL == pool)
		{
			return NULL;
		}
		memset(pool, 0, sizeof(mg_pool_t)*2);		
		mg_queue_init(&(pool->qu));					/*set root pool's parent pool as itself*/
		pool->type = MG_MEM_POOL;						/*set type and size*/
		pool->size = sizeof(mg_pool_t);
		pool++;												/*now point to the data zone, where the root pool struct lies*/
		mem_count += sizeof(mg_pool_t)*2;
	}
	else
	{															/*create sub pool, from its parent pool, of course*/
		pool = (mg_pool_t *)mg_pcalloc(b_pool, sizeof(mg_pool_t));
		
		if(NULL == pool)
		{
			return NULL;
		}
		pool =  pool - 1;									/*pool now point to the mem head*/
		pool->type = MG_MEM_POOL;						/*set type :)  */
		pool->size = sizeof(mg_pool_t);
		pool ++;												/*pool point to the data zone, again*/
	}
	
	
	mg_queue_init(&(pool->qu));						/*here to init the pool we got, so that we can mg_pcalloc later*/

	return pool;
}


/*
从内存池中分配size大小的空间。
实际上依然需要malloc，但是会被添加到pool struct 中的链表中，便于统一释放。
如前述，数据区之前会额外分配头部信息，包含链表信息。
头部信息不可见，也不应该访问，意外访问可能造成内存池释放错误。
数据区并不存在于mg_pool_t的定义中，但是分配空间时，可用空间确实紧跟在其后。
*/
void * mg_pcalloc(mg_pool_t *pool, size_t size)
{
	size_t		all_size = 0;							/*all_size = data_size + mem_head_size*/
	mg_pool_t	*p = NULL;

	if(NULL == pool)
	{
		return NULL;
	}

	all_size = sizeof(mg_pool_t) + size;
	p = malloc(all_size);
	if(NULL == p)
	{
		return NULL;
	}

	memset(p,0,all_size);
	
	p->size = size;
	p->type = MG_MEM_NOR;								/*remember, set the type, never forget that, or pool will kill your sys, maybe :)*/
	mg_queue_insert_tail(&(pool->qu),&(p->qu));	/*add memmory we got to tail of the link, this we call as add to pool, not bad, henw?*/

	mem_count += (size + sizeof(mg_pool_t));

	return p+1;												/*p+1, point to the data zone*/
}



/*
释放pool。
任何有子 pool 的情况下，都会递归释放其下链表中注册的内粗。
*/
void	  mg_pool_destroy(mg_pool_t *pool)
{
	mg_queue_t *sen=NULL;
	mg_queue_t *visit=NULL;
	mg_queue_t *next=NULL;
	int        normem = 0;


	mg_pool_t	*data = NULL;

	if(NULL == pool)										/*no data to free*/
	{
		return;
	}

	sen = &pool->qu;
																/*visit all mem in link*/
	for(visit = sen->next; visit != sen; visit = next)
	{
		next = mg_queue_next(visit);
		data = mg_queue_data(visit, mg_pool_t, qu);
	
		if(data->type == MG_MEM_POOL)					/*if type is POOL, free it*/
		{
			mg_pool_destroy(data+1);
			
		}
		else
		{
			mg_free(data+1);								/*or free mem directly*/
		}

	}


	mg_free(pool);											/*now don't forget to free the root in this level, well, maybe not a real 'root' pool*/

	return;
}

/*
释放指定内存。
这里mem指向的是注册时的内存位置，不能随意释放任意位置。
会从mem处向低地址推测mem head 所在位置。
其操作是将mem处内存冲 mem head 的link 中 剔除，并释放。
*/
void   mg_free(void * mem)								/*easy*/
{
	mg_pool_t *p;
	if(NULL == mem)
	{
		return;
	}
	
	p = mem - sizeof(mg_pool_t);

	if(p->on_free){
		p->on_free(mem);
	}

	mem_count -= ( sizeof(mg_pool_t) + p->size);

	mg_queue_remove(&p->qu);
	
	free((void *)p);
}

void mg_pool_drop(void *mem)
{
	mg_pool_t *p = NULL;
    if(NULL == mem)
    {
        return;
    }

    p = mem - sizeof(mg_pool_t);

    mg_queue_remove(&p->qu);

	mg_queue_init(&p->qu);
}

void mg_pool_add(mg_pool_t *pool, void *mem)
{
	mg_pool_t *mem_h = NULL;
	if(NULL == pool || NULL == mem)
	{
		return;
	}

	mg_pool_drop(mem);	

	mem_h = mem - sizeof(mg_pool_t);

	mg_queue_insert_tail(&(pool->qu), &(mem_h->qu));
}

int mg_pool_set_on_free(void *mem, on_free_callback_t callback){
	mg_pool_t *p = NULL;
    if(NULL == mem)
    {
        return MG_ERROR;
    }

    p = mem - sizeof(mg_pool_t);

	p->on_free = callback;

	return MG_OK;

}
