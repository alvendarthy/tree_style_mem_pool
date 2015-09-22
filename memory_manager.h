#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>

#include "mg_type.h"

#include "mg_pool.h"
#include "mg_array.h"
#include "mg_queue.h"
#include "mg_string.h"
#include "mg_log.h"


#define MG_OK 			0
#define MG_ERROR 		-1
#define MG_DECLINED		-2
#define MG_ERROR_NOR	-3

#define MG_AGAIN	-10


#define mg_memcpy(dst, src, n)   (void) memcpy(dst, src, n)


#endif
