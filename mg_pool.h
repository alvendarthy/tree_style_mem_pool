#ifndef MG_POOL_T
#define MG_POOL_T

#include "mg_queue.h"
#include <stdlib.h>
#include <string.h>


#define MG_MEM_NOR  0
#define MG_MEM_POOL 1


typedef struct
{
	mg_queue_t		qu;
	size_t			size;
	int				type;
}mg_pool_t;


mg_pool_t *mg_pool_create(mg_pool_t	* b_pool);

void	  mg_pool_destroy(mg_pool_t *pool);

void * mg_pcalloc(mg_pool_t *pool, size_t size);

void   mg_free(void * mem);

void mg_pool_drop(void *mem);

void mg_pool_add(mg_pool_t *pool, void *mem);


#endif
