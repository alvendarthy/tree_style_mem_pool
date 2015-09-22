#ifndef MG_ARRAY_H
#define MG_ARRAY_H

#include "memory_manager.h"

typedef struct mg_array_s  mg_array_t;

struct mg_array_s {
    void        *elts;				/*动态数组地址*/
    uint   		 nelts;				/*数组大小*/
    size_t       size;				/*元素大小*/
    uint		 nalloc;			/*预分配大小*/
    mg_pool_t   *pool;				/*内存池*/
};

/*创建动态数组*/
mg_array_t	* mg_array_create(mg_pool_t *pool, int n, size_t size);
/*数组尾部增加一个元素*/
void * mg_array_push(mg_array_t * a);
/*数组尾部增加n个元素*/
void * mg_array_push_n(mg_array_t * a, uint n);




#endif
