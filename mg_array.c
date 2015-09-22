
#include <stdio.h>
#include "mg_array.h"

/*创建动态数组，该数组只读*/
mg_array_t	* mg_array_create(mg_pool_t *p, int n, size_t size)
{
	mg_array_t *a;

	/*分配管理结构*/
	a = mg_pcalloc(p, sizeof(mg_array_t));
	if (a == NULL) {
        return NULL;
	}

	/*预分配可空间*/
	a->elts = mg_pcalloc(p, n * size);
    if (a->elts == NULL) {
        return NULL;
    }

	/*初始化array管理结构*/
	a->nelts = 0;
	a->size = size;
	a->nalloc = n;
	a->pool = p;

	/*返回结构*/
	return a;
}

/*增加array单元*/
void * mg_array_push(mg_array_t * a)
{
	void        *elt, *new;
	size_t       size;
	mg_pool_t  *p;

	if (a->nelts == a->nalloc) {

		/* the array is full */
		size = a->size * a->nalloc;
		p = a->pool;
		
		/* allocate a new array */
		new = mg_pcalloc(p, 2 * size);
		if (new == NULL) {
			return NULL;
		}

		mg_memcpy(new, a->elts, size);
		a->elts = new;
		a->nalloc *= 2;
	}
	
	/*not full, just use it*/
	elt = (u_char *) a->elts + a->size * a->nelts;
	a->nelts++;
	return elt;
}

void * mg_array_push_n(mg_array_t * a, uint n)
{
	void        *elt, *new;
	size_t       size;
	mg_pool_t  *p;

	if(a->nelts+n >= a->nalloc) 
	{

		/* the array is full */
		size = 2 * ((n >= a->nalloc) ? n : a->nalloc);
		p = a->pool;
		
		/* allocate a new array */
		new = mg_pcalloc(p, size * a->size);
		if (new == NULL) {
			return NULL;
		}

		mg_memcpy(new, a->elts, a->nalloc * a->size);
		a->elts = new;
		a->nalloc = size;
	}

	elt = (u_char *) a->elts + a->size * a->nelts;
	a->nelts += n;
	return elt;
}
