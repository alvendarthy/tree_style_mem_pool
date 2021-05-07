#define MG_DEBUG 

#include <stdio.h>

#include "memory_manager.h"

extern long mem_count;

int say_bye(void *m){
	printf("Bye, free mem at %p \n", m);
	return 0;
}


int main(int argc, char **argv)
{
	mg_pool_t   *main_pool =   NULL;
	mg_pool_t   *pool1    =    NULL;
	mg_pool_t   *pool2    =    NULL;
	mg_pool_t   *pool3    =    NULL;
	char         *tmp    =    NULL;
	char         *tmp1    =    NULL;
	char         *tmp2    =    NULL;
	char         *tmp3    =    NULL;
	int rc;

	/* you can display mem_count to see memory cost*/

	/*build root pool*/
	main_pool = mg_pool_create(NULL);
	/*pool1 as a child pool of main_pool*/
	pool1 = mg_pool_create(main_pool);

	/*pool2, build another root pool*/
	pool2 = mg_pool_create(NULL);

	/*pool3 is a root pool, too*/
	pool3 = mg_pool_create(NULL);

	/*let's alloc 1024 byte in every pool*/
	tmp =mg_pcalloc(main_pool, 1024);
	
	mg_pool_set_on_free(tmp, say_bye);

	/*pool1 was a child of main_pool, now pool1 become a root pool*/
	mg_pool_drop(pool1);

	/*
		destroy main_pool. the memory of tmp, which belongs to main_pool,  is freed as well.
		pool1 was a child of main_pool, but it's droped out, so it's still available.
	*/
	mg_pool_destroy(main_pool);


	tmp1= mg_pcalloc(pool1,1024);

	/*tmp1 now a free segment*/
	mg_pool_drop(tmp1);
	/*destroy pool1, but tmp1 was droped out pool1, so tmp1 is still available*/
	mg_pool_destroy(pool1);


	tmp2=mg_pcalloc(pool2,1024);
	tmp3=mg_pcalloc(pool3,1024);

	/*add pool3 as a child pool of pool2*/
	mg_pool_add(pool2,pool3);
	/*
		tmp2 freed.
		pool3( it's a child of pool2 ) is destroyed. tmp3 freed as it belongs to pool3
	*/
	mg_pool_destroy(pool2);

	/*it's a free segement, let's free it secially*/
	mg_free(tmp1);

	return 0;
}
