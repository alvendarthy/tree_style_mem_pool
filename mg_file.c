#include "mg_file.h"
#include "mg_line_format.h"

int mg_file_open(mg_file_t *file)
{
	mg_pool_t	*pool = NULL;

	char		*file_name = NULL;
	char		*mode = NULL;

	mg_log_ex(MG_LOG_DEBUG,"check input..");
	if(NULL == file)
	{
		mg_log_ex(MG_LOG_ERROR, "file null.");
		return MG_ERROR;
	}

	if(0 == file->name.len)
	{
		mg_log_ex(MG_LOG_ERROR, "file name not set yet.");
		return MG_ERROR;
	}

	if(NULL != file->fp)
	{
		mg_log_ex(MG_LOG_ERROR, "file may be already opened.");
		return MG_ERROR;
	}

	if(0 == file->mode.len || file->mode.len > 3)
	{
		mg_log_ex(MG_LOG_ERROR, "file mode illegal");
		return MG_ERROR;
	}
 
	if('r' != file->mode.data[0] &&
	   'w' != file->mode.data[0] &&
	   'a' != file->mode.data[0])
	{
		mg_log_ex(MG_LOG_ERROR, "file mode illegal.");
		return MG_ERROR;
	}

	if(file->mode.len > 1        &&
	   'b' != file->mode.data[1] &&
	   '+' != file->mode.data[1])
	{
		mg_log_ex(MG_LOG_ERROR, "file mode illegal.");
		return MG_ERROR;
	}

	if( 3  == file->mode.len     &&
	   'b' != file->mode.data[1] &&
	   '+' != file->mode.data[1])
	{
		mg_log_ex(MG_LOG_ERROR, "file mode illegal.");
		return MG_ERROR;
	}

	mg_log_ex(MG_LOG_DEBUG, "init file size 0.");
	file->size = 0;

	mg_log_ex(MG_LOG_DEBUG, "int file offset 0.");
	file->offset = 0;

	mg_log_ex(MG_LOG_DEBUG, "create loc pool");
	pool = mg_pool_create(file->parent);
	if(NULL == pool)
	{
		mg_log_ex(MG_LOG_ERROR,"create loc pool error.");
		return MG_ERROR;
	}

	file->pool = pool;

	mg_log_ex(MG_LOG_DEBUG, "open file.");
	file_name = mg_str_to_char(pool, &file->name);
	if(NULL == file_name)
	{
		mg_log_ex(MG_LOG_ERROR, "make file name cpy error.");
		return MG_ERROR;
	}

	mode = mg_str_to_char(pool, &file->mode);
	if(NULL == mode)
	{
		mg_log_ex(MG_LOG_ERROR, "make file mode cpy error.");
		return MG_ERROR;
	}

	file->fp = fopen(file_name, mode);
	if(NULL == file->fp)
	{
		mg_log_ex(MG_LOG_ERROR, "open file failed.");
		return MG_ERROR;
	}

	mg_log_ex(MG_LOG_DEBUG, "check file size.");
	fseek(file->fp, 0L, SEEK_END);
	file->size = ftell(file->fp);
	if(file->size < 0)
	{
		mg_log_ex(MG_LOG_ERROR, "check file size failed");
		mg_file_close(file);
		return MG_ERROR;
	}

	fseek(file->fp, 0L, SEEK_SET);
	return MG_OK;
}

int mg_file_close(mg_file_t *file)
{
	mg_log_ex(MG_LOG_DEBUG, "check input");
	if(NULL == file)
	{
		mg_log_ex(MG_LOG_ERROR, "file null.");
		return MG_ERROR;
	}

	if(NULL == file->fp)
	{
		mg_log_ex(MG_LOG_DEBUG, "fp null.");
		return MG_OK;
	}

	if(NULL == file->pool)
	{
		mg_log_ex(MG_LOG_ERROR, "file loc pool null, this will not rise an error.");
	}

	fclose(file->fp);
	file->fp = NULL;

	mg_pool_destroy(file->pool);

	return MG_OK;
}
