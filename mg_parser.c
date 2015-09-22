#include "memory_manager.h"
#include "mg_parser.h"
#include "mg_file.h"
#include "mg_string.h"


/**
* @brief 解析配置文件中的一行，所有参数均分割到args中。
* @author 马勇
* @date 2013-03-26
*
* @param parser 通过接口创建的解析器结构
* @param args	参数列表
*
* @return MG_OK   配置解析完成
*         MG_AGAIN 继续解析
*/
static int mg_parser_parse_line(mg_parser_t *parser, mg_array_t *args);


/**
* @brief 解析配置块，这里可能发生递归调用，递归层次与配置嵌套层次一致.
* @author 马勇
* @date 2013-03-26
*
* @param parser	通过接口创建的解析器
* @param this	当前配置块对应的结构入口
*
* @return MG_OK  配置块解析成功
*		  MG_ERROR 配置块解析失败
*/
static int mg_parser_block(mg_parser_t *parser, void *this);


/**
* @brief 检查cmd指令类型，cmd中使用char**指定了上一级配置块应当拥有的特性。仅当
*        cmd中pre_type全部在link的type匹配成功，该cmd才能获得匹配。
* @author 马勇
* @date 2013-03-26
*
* @param link	当前配置块parser_link指针，该结构位于当前配置块对应结构的前端
* @param cmd    欲检查的指令
*
* @return MG_OK		匹配成功
*		  MG_DECLINED 匹配失败
*/
static int mg_parser_check_type(mg_parser_link_t *link, mg_parser_cmd_t *cmd);



/**
* @brief 在指定pool中创建解析器并返回。pool不允许为空，是为了防止内存泄露。
*		 解析器位于解析器自身持有的pool中。释放解析器所有内存，只需要释放
*		 其持有的pool即可。
* @author 马勇
* @date 2013-03-26
*
* @param pool 解析器所使用内存池
*
* @return  NULL 创建失败
*		   其他 空白的解析器
*/
mg_parser_t * mg_parser_get(mg_pool_t *pool)
{
	mg_parser_t	*parser = NULL;						/*the parser struct*/
	mg_pool_t	*lpool = NULL;						/*pool for parser*/

	mg_log_ex(MG_LOG_DEBUG, "check input.");
	if(NULL == pool)
	{
		mg_log_ex(MG_LOG_ERROR, "null input.");
		return NULL;
	}

	mg_log_ex(MG_LOG_DEBUG, "create parser.");
	lpool = mg_pool_create(pool);
	if(NULL == lpool)
	{
		mg_log_ex(MG_LOG_ERROR, "create loc pool failed.");
		goto ERROR;
	}

	parser = mg_pcalloc(lpool, sizeof(mg_parser_t));
	if(NULL == parser)
	{
		mg_log_ex(MG_LOG_ERROR, "create parser failed.");
		goto ERROR;
	}

	parser->parent = pool;
	parser->file.parent = lpool;					/*parser file take the parser pool as parent pool*/
	parser->pool = lpool;
	
	mg_log_ex(MG_LOG_DEBUG, "create cmds array.");	/*dynamic array*/
	parser->cmds = mg_array_create(parser->pool, 32, sizeof(mg_parser_cmd_t));
	if(NULL == parser->cmds)
	{
		mg_log_ex(MG_LOG_ERROR, "create cmds array failed.");
		goto ERROR;
	}

	return parser;
ERROR:
	mg_pool_destroy(parser->pool);
	return NULL;
}



/**
* @brief 向解析器中增加指令集，所有指令集都应当以NULL_CMD结束
* @author 马勇
* @date 2013-03-26
*
* @param parser	解析器
* @param cmds	指令集数组，以NULL_CMD结束
*
* @return 非负数   成功条目数
*		  MG_ERROR 失败 
*/
int mg_parser_add_cmds(mg_parser_t *parser, mg_parser_cmd_t *cmds)
{
	mg_parser_cmd_t *pa_cmd = NULL;				/*cmds pushed in the array*/
	mg_parser_cmd_t *in_cmd = NULL;				/*cmds tobe insert*/
	int				nin_cmd = 0;				/*how many cmds be there*/

	mg_log_ex(MG_LOG_DEBUG, "check input");
	if(NULL == parser || NULL == cmds)
	{
		mg_log_ex(MG_LOG_ERROR,"null input.");
		return MG_ERROR;
	}

	in_cmd = cmds;

	for(nin_cmd = 0; 0 != in_cmd[nin_cmd].key.len ||		/*not NULL_CMD*/
					 0 != in_cmd[nin_cmd].nargs   ||
					 NULL != in_cmd[nin_cmd].type ||
					 NULL != in_cmd[nin_cmd].pre_type ||
					 NULL != in_cmd[nin_cmd].create; nin_cmd ++);	/*count cmds*/

	mg_log_ex(MG_LOG_DEBUG, "push into parser cmds.");
	pa_cmd = mg_array_push_n(parser->cmds, nin_cmd);
	if(NULL == pa_cmd)
	{
		mg_log_ex(MG_LOG_ERROR, "push into parser cmds failed.");
		return MG_ERROR;
	}

	memcpy(pa_cmd, in_cmd, sizeof(mg_parser_cmd_t)*nin_cmd);	/*move mem*/

	return nin_cmd;
}


/**
* @brief 设置配置文件
* @author 马勇
* @date 2013-03-26
*
* @param parser	解析器
* @param file	配置文件
*
* @return MG_OK	成功
*		  MG_ERROR 失败
*/
int	mg_parser_set_file(mg_parser_t *parser, mg_str_t *file)
{
	mg_pool_t	*pool = NULL;				/*pool for conf file*/
	mg_str_t	file_name;					/*file name*/

	mg_log_ex(MG_LOG_DEBUG, "check imput");
	if(NULL == parser || NULL == file)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput.");
		return MG_ERROR;
	}
	pool = parser->pool;
	if(NULL == pool)
	{
		mg_log_ex(MG_LOG_ERROR, "parser pool null.");
		return MG_ERROR;
	}

	file_name.len = file->len;
	file_name.data = mg_str_to_char(pool, file);
	if(NULL == file_name.data)
	{
		mg_log_ex(MG_LOG_ERROR, "make file name cpy failed.");
		return MG_ERROR;
	}

	parser->file.name = file_name;

	return MG_OK;
}


/**
* @brief 设置配置文件， 使用C string作参数
* @author 马勇
* @date 2013-03-26
*
* @param parser
* @param file
*
* @return MG_OK 成功
*		  MG_ERROR 失败
*/
int	mg_parser_set_file_c(mg_parser_t *parser, char *file)
{
	mg_pool_t	*pool = NULL;
	mg_str_t	file_name;

	mg_log_ex(MG_LOG_DEBUG, "check imput");
	if(NULL == parser || NULL == file)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput.");
		return MG_ERROR;
	}
	pool = parser->pool;
	if(NULL == pool)
	{
		mg_log_ex(MG_LOG_ERROR, "parser pool null.");
		return MG_ERROR;
	}

	file_name.len = strlen(file);
	file_name.data = mg_pcalloc(pool, file_name.len+1);
	if(NULL == file_name.data)
	{
		mg_log_ex(MG_LOG_ERROR, "make file name cpy failed.");
		return MG_ERROR;
	}

	memcpy(file_name.data, file, file_name.len);

	parser->file.name = file_name;

	return MG_OK;
}



/**
* @brief  初始化解析器，准备配置文件等。
* @author 马勇
* @date 2013-03-26
*
* @param parser	解析器
*
* @return MG_OK	成功
*		  MG_ERROR 失败
*/
int mg_parser_init(mg_parser_t *parser)
{
	int	rc = 0;
	mg_file_t	*file = NULL;

	mg_log_ex(MG_LOG_DEBUG, "check imput.");
	if(NULL == parser)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput.");
		return MG_ERROR;
	}

	if(NULL == parser->cmds)
	{
		mg_log_ex(MG_LOG_ERROR, "no cmds found!");
		return MG_ERROR;
	}
	
	if(0 == parser->file.name.len)
	{
		mg_log_ex(MG_LOG_ERROR, "conf file not set yet.");
		return MG_ERROR;
	}

	file = &parser->file;

	mg_str_set(&file->mode, FILE_O_RD);

	rc = mg_file_open(file);
	if(MG_OK != rc)
	{
		mg_log_ex(MG_LOG_ERROR, "cannot open the conf file.");
		return MG_ERROR;
	}
	
	return MG_OK;
}



/**
* @brief 解析配置文件
* @author 马勇
* @date 2013-03-26
*
* @param parser	解析器
* @param root	用于接受解析结果的root结构体
*
* @return MG_OK 成功
*		  MG_ERROR 失败
*/
int mg_parser_run(mg_parser_t *parser, mg_conf_root_t **root)
{
	mg_conf_root_t *conf = NULL;			/*解析成功的配置文件，输出到(*root)->data*/
	mg_pool_t		*parser_pool = NULL;	/*解析器的内存池*/
	mg_pool_t		*conf_pool = NULL;		/*配置内存池*/

	int		rc = 0;							/*functions' return values*/
//	int		i = 0;							/*loop flag*/
//	int		matched = 0;					/*cmd matched*/

	char		*root_t[]={"sroot", "block", NULL};	/*由框架创建，root结构的类型为 supper root, block*/


	mg_log_ex(MG_LOG_DEBUG, "check imput.");
	if(NULL == parser || NULL == root)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput");
		return MG_ERROR;
	}

	parser_pool = parser->pool;
	conf_pool = mg_pool_create(parser_pool);
	if(NULL == conf_pool)
	{
		mg_log_ex(MG_LOG_ERROR, "create conf pool failed.");
		return MG_ERROR;
	}

	conf = mg_pcalloc(conf_pool, sizeof(mg_conf_root_t));
	if(NULL == conf)
	{
		mg_log_ex(MG_LOG_ERROR, "create conf struct failed.");
		goto ERROR;
	}

	conf->mg_parser_link.pool = conf_pool;
	conf->mg_parser_link.parent = parser_pool;
	conf->mg_parser_link.this = conf;
	conf->mg_parser_link.is_sroot = 1;
	conf->mg_parser_link.type = root_t;

	*root = conf;								/*out put*/

	rc = mg_parser_block(parser, conf);			/*begin to parse the root block*/
	if(MG_OK != rc)
	{
		mg_log_ex(MG_LOG_ERROR, "parse block failed.");
		goto ERROR;
	}


	return MG_OK;

ERROR:
	mg_pool_destroy(conf_pool);
	return MG_ERROR;
}



/**
* @brief 解析配置块，所谓配置块，由cmd->type中的block关键字指定.
* @author 马勇
* @date 2013-03-26
*
* @param parser	解析器
* @param this	当前配置块所对应的结构体入口
*
* @return	MG_OK 成功
*		    MG_ERROR 失败
*/
static int mg_parser_block(mg_parser_t *parser, void *this)
{
	mg_parser_link_t *link = this;			/*当前配置块的link struct，其包含配置层次数据*/
	mg_parser_link_t *new = NULL;			/*新创建下层配置块结构入口*/
	mg_array_t	*args = NULL;				/*已解析获得的参数们*/
	mg_pool_t	*pool = NULL;				/*工作所在内存池*/
	int		rc = 0;							/*return values*/
	int		i = 0;							/*block*/
	int		matched = 0;

	char	**new_type = NULL;				/*新创建配置块类型信息*/

	enum									/*配置块控制*/
	{
		NON,								/*不期待任何特殊符号*/
		BLOCK_BGN,							/*期待 ｛*/
		BLOCK_END							/*期待 ｝*/
	}expect;

	int			ncmds = 0;					/*cmd 数量*/
	mg_parser_cmd_t *cmd = NULL;			/*cmd 数组*/

	mg_str_t	*arg;						/*参数数组*/
	void		*new_data = NULL;			/*新建数据*/

	mg_log_ex(MG_LOG_DEBUG, "check imput");
	if(NULL == parser || NULL == this)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput");
		return MG_ERROR;
	}
	
	pool = link->pool;						/*使用当前配置块的pool*/

	ncmds = parser->cmds->nelts;			/*cmd数据*/
	cmd  = parser->cmds->elts;

	if(link->is_sroot)						/*对supper root不需要｛｝*/
	{
		expect = NON;
	}
	else
	{
		expect = BLOCK_BGN;					/*其余配置块需要使用 ｛｝*/
	}

	for(;;)
	{
		args = mg_array_create(pool, 4, sizeof(mg_str_t ));	/*创建参数列表*/
		if(NULL == args)
		{
			mg_log_ex(MG_LOG_ERROR, "create args failed");
			return MG_ERROR;
		}
		rc = mg_parser_parse_line(parser, args);
		if(MG_ERROR == rc)
		{
			mg_log_ex(MG_LOG_ERROR, "parse line failed.");
			return MG_ERROR;
		}

		if(MG_OK == rc)
		{
			if(BLOCK_BGN == expect)
			{
				mg_log_ex(MG_LOG_EMERG, "expecting {, line %d", parser->line);
				return MG_ERROR;
			}

			if(BLOCK_END == expect)
			{
				mg_log_ex(MG_LOG_EMERG, "expecting }, line %d", parser->line);
				return MG_ERROR;
			}

			return MG_OK;
		}
		
		if(0 == args->nelts)
		{
			continue;
		}

		arg = args->elts;

		if(!link->is_sroot)
		{
			if(args->nelts == 1)
			{
				if(mg_str_eq_char(arg, "{"))
				{
					if(expect != BLOCK_BGN)
					{
						mg_log_ex(MG_LOG_EMERG, "unexpected {, line %d", parser->line);
						return MG_ERROR;
					}
					expect = BLOCK_END;
					continue;
				}
		
				if(mg_str_eq_char(arg, "}"))
				{
					if(expect != BLOCK_END)
					{
						mg_log_ex(MG_LOG_EMERG, "unexpected }, line %d", parser->line);
						return MG_ERROR;
					}

					expect = NON;
					return MG_OK;
				}
			}
			
			if(expect != BLOCK_END)
			{
				mg_log_ex(MG_LOG_EMERG, "expecting {, line %d", parser->line);
				return MG_ERROR;
			}
		}
		else
		{
			if(1 == args->nelts && (mg_str_eq_char(arg, "{") || mg_str_eq_char(arg, "}")))
			{
				mg_log_ex(MG_LOG_EMERG, "unexpected } or {, line %d", parser->line);
				return MG_ERROR;
			}
		}

		matched = 0;
		
		for(i = 0; i < ncmds; i ++)
		{
			if(!mg_str_eq_str(&arg[0], &cmd[i].key))
			{
				continue;
			}

			rc = mg_parser_check_type(link, &cmd[i]);
			if(MG_OK != rc)
			{
				continue;
			}

			if(args->nelts != (cmd[i].nargs+1))
			{
				mg_log_ex(MG_LOG_EMERG, "expect %d args, line %d.", cmd[i].nargs, parser->line);
				return MG_ERROR;
			}


			matched = 1;
			break;
		}

		if(matched != 1)
		{
			mg_log_ex(MG_LOG_EMERG, "undefined cmd %s, line %d", arg[0].data, parser->line);
			return MG_ERROR;
		}

		new_data = NULL;
		new_type = NULL;

		if(NULL != cmd[i].create)
		{
			
			new_data = cmd[i].create(this, cmd[i].create_offset, cmd[i].size);
			if(NULL == new_data)
			{
				return MG_ERROR;
			}
		}

		if(NULL != new_data)
		{
			new = new_data;
			new->pre = this;
			new->this = new_data;
			new->is_sroot = 0;
			new->pool = link->pool;
			new->parent = link->parent;
			new->type = cmd[i].type;
			new_type = new->type;
		}
	
		if(NULL != cmd[i].parse )
		{
			rc = cmd[i].parse(this, new_data, cmd[i].parse_offset, args);
			if(MG_OK != rc)
			{
				mg_log_ex(MG_LOG_EMERG, "parse cmd failed. line %d", parser->line);
				return MG_ERROR;
			}
		}

		if(NULL != new_type)
		{
			for(;NULL != new_type && NULL != *(new_type); new_type ++)
			{
				if(mg_char_eq_char(*new_type, "block"))			/*if new block created*/
				{
					rc = mg_parser_block(parser, new);			/*parse it*/
					if(MG_OK != rc)	
					{
						mg_log_ex(MG_LOG_ERROR, "parse block failed. line %d", parser->line);
						return MG_ERROR;
					}
					break;
				}
			}
		}
	}
}


static int mg_parser_check_type(mg_parser_link_t *link, mg_parser_cmd_t *cmd)
{
	char **link_type = NULL;
	char **cmd_pre_type = NULL;

	for(cmd_pre_type = cmd->pre_type; NULL != cmd_pre_type && NULL != *cmd_pre_type; cmd_pre_type ++)
	{
		for(link_type = link->type; NULL != link_type && NULL != *link_type; link_type ++)
		{
			if(mg_char_eq_char(*link_type, *cmd_pre_type))
			{
				goto NEXT_CMD_TYPE;
			}
		}
		return MG_DECLINED;

NEXT_CMD_TYPE:
		continue;
	}

	return MG_OK;
}


int mg_parser_drop_conf_mem(mg_conf_root_t *root)
{
	mg_log_ex(MG_LOG_DEBUG, "check imput.");
	if(NULL == root)
	{
		mg_log_ex(MG_LOG_ERROR, "null root conf.");
		return MG_ERROR;
	}

	mg_pool_drop(root->mg_parser_link.pool);
	return MG_OK;
}

int	mg_parser_close(mg_parser_t *parser)
{
	int	rc = 0;
	mg_log_ex(MG_LOG_DEBUG, "check imput.");
	if(NULL == parser)
	{
		mg_log_ex(MG_LOG_ERROR, "null parser.");
		return MG_ERROR;
	}

	rc = mg_file_close(&parser->file);
	if(MG_OK != rc)
	{
		mg_log_ex(MG_LOG_ERROR, "close conf file failed.");
		return MG_ERROR;
	}

	mg_pool_destroy(parser->pool);

	return MG_OK;
}


typedef enum
{
	NON,
	LOOK_WORD_OR_QUOT_BGN,
	LOOK_WORD_BGN,
	LOOK_WORD_END,
	LOOK_QUOT_END,
	COMENT
}parse_phase_t;


static int mg_parser_parse_line(mg_parser_t *parser, mg_array_t *args)
{
	FILE *fp = NULL;
	mg_pool_t	*args_pool = NULL;
	char	buf[INIT_BUF_DEF_SIZE];

	mg_str_t	*new_arg = NULL;
	
	char	*scanner = NULL;

	char	*str_bgn = NULL;
	char	*str_end = NULL;
	int		slen = 0;
	int		dec_len = 0;

	int		copy_i = 0;
	
	parse_phase_t	phase = NON;
	int				found = 0;
	int				push_new = 0;
	int				comment = 0;
	int				line_end = 0;
	int				next_line = 0;

	mg_log_ex(MG_LOG_DEBUG, "check imput.");
	if(NULL == parser || NULL == args)
	{
		mg_log_ex(MG_LOG_ERROR, "null imput.");
		return MG_ERROR;
	}

	args_pool = args->pool;
	if(NULL == args_pool)
	{
		mg_log_ex(MG_LOG_ERROR, "args pool null.");
		return MG_ERROR;
	}

	fp = parser->file.fp;
	if(NULL == fp)
	{
		mg_log_ex(MG_LOG_ERROR, "file not opened!");
		return MG_ERROR;
	}
	
	for(;!feof(fp);)
	{
		memset(buf, 0, INIT_BUF_DEF_SIZE);

		scanner = fgets(buf, INIT_BUF_DEF_SIZE-1, fp);/*get line*/
		if(NULL == scanner)
		{
			mg_log_ex(MG_LOG_DEBUG, "quit, may file read EOF or error.");
			return MG_OK;
		}

		parser->line ++;

		/*init flags*/
		phase = LOOK_WORD_OR_QUOT_BGN;
		found = 0;
		line_end = 0;
		comment = 0;
		next_line = 1;
		dec_len = 0;

		str_bgn = buf;
		str_end = buf;

		while(1)
		{
			push_new = 0;

			if(*scanner == '\\')
			{
				scanner += 2;
				dec_len ++;
			}

			if(*scanner == '\n' || *scanner == '\0')
			{
				if(phase == LOOK_QUOT_END)
				{
					mg_log_ex(MG_LOG_EMERG, "unexpected end. line %d", parser->line);
					return MG_ERROR;
				}

				line_end = 1;
			}

			if(*scanner == '#')
			{
				comment = 1;
			}

			switch(phase)
			{
				case NON:
					phase = LOOK_WORD_OR_QUOT_BGN;
					break;
				case LOOK_WORD_OR_QUOT_BGN:
					if(comment == 1)
					{
						break;
					}

					if(*scanner == '"')
					{
						phase = LOOK_QUOT_END;
						found = 1;
						scanner ++;
						str_bgn = scanner;
						break;
					}

					if((*scanner >= 'a' && *scanner <= 'z') ||
					   (*scanner >= 'A' && *scanner <= 'Z') ||
					   (*scanner >= '0' && *scanner <= '9') ||
					    *scanner == '_' ||
						*scanner == '}' ||
						*scanner == '{')
					{
						phase = LOOK_WORD_END;
						found = 1;
						str_bgn = scanner;
						scanner ++;
						break;
					}

					if(*scanner != ' ' && *scanner != '\t' && *scanner != '\n')
					{
						mg_log_ex(MG_LOG_EMERG, "unexpected character %s LINE%d", scanner, parser->line);
						return MG_ERROR;
					}

					if(line_end == 0)
					{
						scanner ++;
					}
					break;
				case LOOK_WORD_BGN:
					break;
				case LOOK_WORD_END:
					if(comment == 1)
					{
						str_end = scanner;
						//scanner ++;
						push_new = 1;
						break;
					}

					if((*scanner >= 'a' && *scanner <= 'z') ||
					   (*scanner >= 'A' && *scanner <= 'Z') ||
					   (*scanner >= '0' && *scanner <= '9') ||
					    *scanner == '_' || 
						*scanner == '}' ||
						*scanner == '{')
					{
						scanner ++;
						break;
					}

					str_end = scanner;
					//scanner ++;
					push_new = 1;
					break;
				case LOOK_QUOT_END:
					comment = 0;
					if(*scanner != '"')
					{
						scanner ++;
						break;
					}

					str_end = scanner;
					scanner ++;
					push_new = 1;

					break;
				case COMENT:
					
					break;

				default:
					mg_log_ex(MG_LOG_EMERG, "unexpected phase!");
					return MG_ERROR;
					break;
			}

			if(push_new)
			{
				slen = str_end - str_bgn;
/*				if(0 == slen)
				{
					mg_log_ex(MG_LOG_DEBUG, "arg len 0, skip this.");
					phase = LOOK_WORD_OR_QUOT_BGN;
					goto AN_ARG_DONE;		
				}
*/
				new_arg = mg_array_push(args);
				if(NULL == new_arg)
				{
					mg_log_ex(MG_LOG_ERROR, "push new arg failed.");
					return MG_ERROR;
				}

				new_arg->len = slen - dec_len;
				new_arg->data = mg_pcalloc(args_pool, slen+1);
				if(NULL == new_arg)
				{
					mg_log_ex(MG_LOG_ERROR, "alloc arg failed.");
					return MG_ERROR;
				}

				//memcpy(new_arg->data, str_bgn, slen);
				for(copy_i = 0; str_bgn < str_end; str_bgn ++)
				{
					if('\\' == *str_bgn)
					{
						if(str_bgn+1 < str_end)
                        {
                            str_bgn ++;
                            new_arg->data[copy_i] = *str_bgn;
                            copy_i ++;
                        }
						continue;
					}
					new_arg->data[copy_i] = *str_bgn;
					copy_i ++;
				}

				next_line = 0;
				phase = LOOK_WORD_OR_QUOT_BGN;
				dec_len = 0;
			}
AN_ARG_DONE:
			if(comment == 1 || line_end == 1)
			{
				break;
			}
		}

		if(next_line == 0)
		{
			return MG_AGAIN;
		}
		
		continue;
	}

	return MG_OK;
}

void *mg_parser_def_create(void *pre, int offset, int size)
{
	mg_parser_link_t *link = pre;
	mg_pool_t *pool = link->pool;

	void	* conf = NULL;
	char	** save = NULL;

	save = (char **)((char *)pre + offset);

	conf = mg_pcalloc(pool, size);
	if(NULL == conf)
	{
		return conf;
	}

	*save = conf;

	return conf;
}

void *mg_parser_array_create(void *pre, int offset, int size)
{
    mg_parser_link_t *link = pre;
    mg_pool_t *pool = link->pool;
	mg_array_t  *array = NULL;

    void    * conf = NULL;
    char    ** save = NULL;

    save = (char **)((char *)pre + offset);

	if(NULL == *save)
	{
		array = mg_array_create(pool, MG_PARSER_ARRAY_DEF_COUNT, size);
		if(NULL == array)
		{
			return NULL;
		}

		*save = array;
	}

    conf = mg_array_push(*save);
    if(NULL == conf)
    {
        return NULL;
    }

    return conf;
}

int	mg_parser_str_slot(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	mg_str_t			*arg = cmd->elts;
	mg_str_t			*tar = (mg_str_t *)((char*)pre + offset);
	
	*tar = arg[1];
	
	return MG_OK;
}

int	mg_parser_flag_slot(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	mg_str_t			*arg = cmd->elts;
	int				*tar = (int*)((char*)pre + offset);
	
	if(mg_str_eq_char(&arg[1], "on"))
	{
		*tar = 1;
		return MG_OK;
	}
	
	if(mg_str_eq_char(&arg[1], "off"))
	{
		*tar = 0;
		return MG_OK;
	}

	mg_log_ex(MG_LOG_EMERG, "expecting arg \"on\" or \"off\".");
	
	return MG_ERROR;
}

int	mg_parser_num_slot(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	mg_str_t			*arg = cmd->elts;
	int				*tar = (int *)((char*)pre + offset);
	
	*tar = atoi(arg[1].data);
	
	return MG_OK;
}
