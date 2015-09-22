#ifndef MG_PARSER_H
#define MG_PARSER_H

#include "memory_manager.h"
#include "mg_file.h"

#define INIT_BUF_DEF_SIZE 2048

#define MG_PARSER_ARRAY_DEF_COUNT 8

#define MG_PARSER_LINK  mg_parser_link_t mg_parser_link

#define MG_PARSER_NULL_CMD {{0,0}, 0, NULL, NULL, NULL, 0, 0, NULL, 0}

typedef struct
{
	void		*pre;
	void		*this;
	int			is_sroot;
	mg_pool_t	*pool;
	mg_pool_t	*parent;
	char		**type;
}mg_parser_link_t;

typedef struct
{
	MG_PARSER_LINK;
	void *data;
}mg_conf_root_t;



typedef struct
{
	mg_str_t	key;					/*自动添加到type中*/
	int		nargs;
	char		**type;
	char		**pre_type;
	void	*(*create)(void *pre, int offset, int size);
	int		create_offset;
	int		size;
	int	(*parse)(void *pre, void *conf, int offset, mg_array_t *cmd);
	int		parse_offset;
}mg_parser_cmd_t;

typedef struct
{
	mg_file_t		file;
	int				line;
	mg_conf_root_t	root;
	mg_array_t		*cmds;
	mg_pool_t		*pool;
	mg_pool_t		*parent;
}mg_parser_t;

/*获得一个空白的parser*/
mg_parser_t	* mg_parser_get(mg_pool_t *pool);
/*向parser中增加cmd，返回增加cmd数目*/
int			  mg_parser_add_cmds(mg_parser_t *parser, mg_parser_cmd_t *cmds);
int			  mg_parser_set_file(mg_parser_t *parser, mg_str_t	*file);
int			  mg_parser_set_file_c(mg_parser_t *parser, char *file);
int			  mg_parser_init(mg_parser_t *parser);
int			  mg_parser_run(mg_parser_t *parser, mg_conf_root_t **root);
int			  mg_parser_drop_conf_mem(mg_conf_root_t *root);
int			  mg_parser_close(mg_parser_t *parser);

void *mg_parser_def_create(void *pre, int offset, int size);
void *mg_parser_array_create(void *pre, int offset, int size);
int	mg_parser_str_slot(void *pre, void *conf, int offset, mg_array_t *cmd);
int	mg_parser_flag_slot(void *pre, void *conf, int offset, mg_array_t *cmd);
int	mg_parser_num_slot(void *pre, void *conf, int offset, mg_array_t *cmd);

#endif
