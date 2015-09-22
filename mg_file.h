#ifndef MG_FILE_H
#define MG_FILE_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "memory_manager.h"

#define FILE_O_RD	"r"
#define FILE_O_RDB	"rb"
#define FILE_O_WT	"w"
#define FILE_O_WTB	"wb"
#define FILE_O_AP	"a"
#define FILE_O_APB	"ab"
#define FILE_O_RD_P		"r+"
#define FILE_O_RDB_P	"rb+"
#define FILE_O_WT_P		"w+"
#define FILE_O_WTB_P	"wb+"


typedef struct
{
	mg_str_t	name;
	FILE		*fp;
	mg_str_t	mode;
	int			size;
	int			offset;
	mg_str_t	err;
	mg_pool_t	*pool;
	mg_pool_t	*parent;
}mg_file_t;


int	mg_file_open(mg_file_t *file);
int	mg_file_close(mg_file_t *file);
#endif
