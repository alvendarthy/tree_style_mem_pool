#include "memory_manager.h"
#include "mg_parser.h"
#include "parser_cmd.h"
#include "mg_line_format.h"

static void    *global_create(void *pre, int offset, int size);
static int		global_set_refer_value_parse(void *pre, void *conf, int offset, mg_array_t *cmd);


static void    *tab_create(void *pre, int offset, int size);
static int		tab_parse(void *pre, void *conf, int offset, mg_array_t *cmd);

static int      tab_set(void *pre, void *conf, int offset, mg_array_t *cmd);

static void    *sql_egn_create(void *pre, int offset, int size);

static int		sql_egn_set_break(void *pre, void *conf, int offset, mg_array_t *cmd);

static int		dynamic_set_tab_cast(void *pre, void *conf, int offset, mg_array_t *cmd);
static int      dynamic_set(void *pre, void *conf, int offset, mg_array_t *cmd);


char *sroot_ty[] = {"sroot", "block", NULL};
char *conf_ty[] = {"conf",  "block", NULL};
char *sql_conf_ty[] = {"sql_conf", "block", NULL};
char *global_ty[] = {"global", "block", NULL};
char *tab_ty[] = {"tab",	"block", NULL};
char *sql_egn_ty[] = {"sql_egn", "block", NULL};
char *dynamic_refer_key_ty[] = {"dynamic_refer_key", "block", NULL};


mg_parser_cmd_t cmds[] =
{
	{
		mg_string("CONF"),
		0,
		conf_ty,
		sroot_ty,
		mg_parser_def_create,
		offsetof(mg_conf_root_t, data),
		sizeof(C_conf_t),
		NULL,
		0
	},

	{
        mg_string("SQL_CONF"),
        0,
        sql_conf_ty,
        conf_ty,
        mg_parser_def_create,
        offsetof(C_conf_t, sql_conf),
        sizeof(C_sql_conf_t),
        NULL,
        0
    },
	{
        mg_string("HOST"),
        1,
        NULL,
        sql_conf_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_conf_t, host)
    },
	{
        mg_string("PORT"),
        1,
        NULL,
        sql_conf_ty,
        NULL,
        0,
        0,
        mg_parser_num_slot,
        offsetof(C_sql_conf_t, port)
    },
	{
        mg_string("USER"),
        1,
        NULL,
        sql_conf_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_conf_t, user)
    },
	{
        mg_string("PASSWD"),
        1,
        NULL,
        sql_conf_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_conf_t, passwd)
    },
	{
        mg_string("DATABASE"),
        1,
        NULL,
        sql_conf_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_conf_t, db)
    },

	{
        mg_string("GLOBAL"),
        0,
        global_ty,
        conf_ty,
        mg_parser_def_create,
        offsetof(C_conf_t, global),
        sizeof(C_global_t),
		NULL,
        0
    },
	{
        mg_string("SET_TIME_FORMAT"),
        1,
        NULL,
        global_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_global_t, time_format)
    },
	{
        mg_string("SET_REFER_VALUE"),
        2,
        NULL,
        global_ty,
        NULL,
        0,
        0,
        global_set_refer_value_parse,
        0
    },

	{
        mg_string("TAB"),
        1,
        tab_ty,
        conf_ty,
        tab_create,
        0,
        0,
        tab_parse,
        0
    },
	{   
        mg_string("SET"),
        2,
        NULL,
        tab_ty,
        NULL,
        0,
        0,
        tab_set,
        0
    },

	{
        mg_string("SQL_EGN"),
        0,
        sql_egn_ty,
        conf_ty,
        sql_egn_create,
        0,
        0,
        NULL,
        0
    },
	{
        mg_string("ID_STR"),
        1,
        NULL,
        sql_egn_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_egn_t, id_str)
    },
	{
        mg_string("SEPER_STR"),
        1,
        NULL,
        sql_egn_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_egn_t, seper)
    },
	{
        mg_string("SIGNER_STR"),
        1,
        NULL,
        sql_egn_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_egn_t, signer)
    },
	{
        mg_string("SQL_QUERY"),
        1,
        NULL,
        sql_egn_ty,
        NULL,
        0,
        0,
        mg_parser_str_slot,
        offsetof(C_sql_egn_t, sql_query)
    },
	{   
        mg_string("DYNAMIC_REFER_KEY"),
        0,
        dynamic_refer_key_ty,
        sql_egn_ty,
        mg_parser_def_create,
        offsetof(C_sql_egn_t, refers),
        sizeof(C_dynamic_refer_t),
        NULL,
        0
    },
	{
        mg_string("SET_TAB_CAST"),
        2,
        NULL,
        dynamic_refer_key_ty,
        NULL,
        0,
        0,
        dynamic_set_tab_cast,
        0
	},
	{   
        mg_string("SET"),
        1,
        NULL,
        dynamic_refer_key_ty,
        NULL,
        0,
        0,
        dynamic_set,
        0
    },
	{
        mg_string("BREAK"),
        0,
        NULL,
        sql_egn_ty,
        NULL,
        0,
        0,
        sql_egn_set_break,
        0
    },

	MG_PARSER_NULL_CMD
};


/*
static void    *global_create(void *pre, int offset, int size)
{
	C_conf_t	*conf = pre;
	C_global_t	*global = NULL;
	mg_parser_link_t	*link = pre;
	mg_pool_t	*pool = link->pool;

	global = mg_pcalloc(pool, sizeof(C_global_t));
	if(NULL == global)
	{
		mg_log_ex(MG_LOG_EMERG, "alloc global failed.");
		return NULL;
	}

	global->refer_value_pairs = mg_array_create(pool, 8, sizeof(mg_str_pair_t));
	if(NULL == global->refer_value_pairs)
	{
		mg_log_ex(MG_LOG_EMERG, "alloc global refer array failed.");
        return NULL;
	}

	conf->global = global;

	return global;
}
*/

static int global_set_refer_value_parse(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_global_t  *global = pre;
    mg_parser_link_t    *link = pre;
    mg_str_pair_t       *pair = NULL;
	mg_str_t			*arg = cmd->elts;
	mg_pool_t			*pool = link->pool;

    if(NULL == global->refer_value_pairs)
    {
		global->refer_value_pairs = mg_array_create(pool, 8, sizeof(mg_str_pair_t));
    	if(NULL == global->refer_value_pairs)
    	{
        	mg_log_ex(MG_LOG_EMERG, "alloc global refer array failed.");
        	return MG_ERROR;
    	}
    }

    pair = mg_array_push(global->refer_value_pairs);
    if(NULL == pair)
    {
        mg_log_ex(MG_LOG_EMERG, "global refer value pairs push failed.");
        return MG_ERROR;
    }

	pair->left = arg[1];
	pair->right = arg[2];

    return MG_OK;
}

static void    *tab_create(void *pre, int offset, int size)
{
	C_conf_t    *conf = pre;
    C_tab_t  	*tab = NULL;
    mg_parser_link_t    *link = pre;
    mg_pool_t   *pool = link->pool;
	mg_array_t	*tabs = NULL;

	if(NULL == conf->tabs)
	{
		tabs = mg_array_create(pool, 4, sizeof(C_tab_t));
		if(NULL == tabs)
		{
			mg_log_ex(MG_LOG_EMERG, "create tab array failed.");
        	return NULL;
		}

		conf->tabs = tabs;
	}

	tab	= mg_array_push(conf->tabs);
	if(NULL == tab)
	{
		mg_log_ex(MG_LOG_EMERG, "tab array push failed.");
        return NULL;
	}

	tab->shadow_pairs = mg_array_create(pool, 8, sizeof(mg_str_pair_t));
	if(NULL == tab->shadow_pairs)
	{
		mg_log_ex(MG_LOG_EMERG, "create tab pairs failed.");
        return NULL;
	}


	return tab;
}


static int      tab_parse(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_tab_t     *tab = conf;
	mg_str_t	*arg = cmd->elts;

	tab->name = arg[1];

	return MG_OK;
}

static int      tab_set(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_tab_t     *tab = pre;
	mg_str_pair_t *pair = NULL;
	mg_str_t    *arg = cmd->elts;

	if(NULL == tab->shadow_pairs)
	{
		mg_log_ex(MG_LOG_EMERG, "tab shadow array not set.");
		return MG_ERROR;
	}

	pair = mg_array_push(tab->shadow_pairs);
	if(NULL == pair)
	{
		mg_log_ex(MG_LOG_EMERG, "tab shadow array push failed.");
        return MG_ERROR;
	}

	pair->left = arg[1];
	pair->right = arg[2];

	return MG_OK;
}


static void    *sql_egn_create(void *pre, int offset, int size)
{
	C_conf_t    *conf = pre;
    C_sql_egn_t  *sql_egn = NULL;
    mg_parser_link_t    *link = pre;
    mg_pool_t   *pool = link->pool;
    mg_array_t  *sql_egns = NULL;

	if(NULL == conf->sql_egn)
	{
		sql_egns = mg_array_create(pool, 4, sizeof(C_sql_egn_t));
		if(NULL == sql_egns)
		{
			mg_log_ex(MG_LOG_EMERG, "sql egn array create error.");
			return NULL;
		}

		conf->sql_egn = sql_egns;
	}
	
	sql_egn = mg_array_push(conf->sql_egn);
	if(NULL == sql_egn)
	{
		mg_log_ex(MG_LOG_EMERG, "sql egn array push error.");
        return NULL;
	}

	return sql_egn;
}

static int      sql_egn_set_break(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_sql_egn_t  *sql_egn = pre;
	
	sql_egn->skip = 1;

	return MG_OK;
}

static int      dynamic_set_tab_cast(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_dynamic_refer_t   *refers = pre;
	mg_parser_link_t	*link = pre;
	C_sql_egn_t			*egn_block = link->pre;

	C_conf_t			*conf_block = egn_block->mg_parser_link.pre;
	C_tab_t				*tab = NULL;
	int					i = 0;	
	int					ntab = 0;
	int					found = 0;

	mg_pool_t			*pool = link->pool;
	C_dynamic_refer_key_t	*key = NULL;
	mg_str_t			*arg = cmd->elts;

	if(NULL == refers->keys)
	{
		refers->keys = mg_array_create(pool, 8, sizeof(C_dynamic_refer_key_t));
		if(NULL == refers->keys)
		{
			mg_log_ex(MG_LOG_EMERG, "create dynamic refers array failed.");
			return MG_ERROR;
		}
	}

	tab = conf_block->tabs->elts;
    ntab = conf_block->tabs->nelts;
	
	if(0 == ntab)
	{
		mg_log_ex(MG_LOG_EMERG, "no tables found, you need to define one with cmd \"TAB name {}\"");
		return MG_ERROR;
	}

	for(i = 0; i < ntab; i ++)
	{
		if(mg_str_eq_str(&tab[i].name, &arg[2]))
		{
			found = 1;
			break;
		}
	}

	if(!found)
	{
		mg_log_ex(MG_LOG_EMERG, "table: %s not found, you need to define one with cmd \"TAB name {}\"", arg[2].data);
        return MG_ERROR;
	}

	key = mg_array_push(refers->keys);
	if(NULL == key)
	{
		mg_log_ex(MG_LOG_EMERG, "dynamic refers array push failed.");
        return MG_ERROR;
	}

	key->key = arg[1];
	key->tab_name = arg[2];
	key->tab = &tab[i];
	key->need_cast = 1;

	return MG_OK;
}


static int      dynamic_set(void *pre, void *conf, int offset, mg_array_t *cmd)
{
	C_dynamic_refer_t   *refers = pre;
    mg_parser_link_t    *link = pre;
    mg_pool_t           *pool = link->pool;
    C_dynamic_refer_key_t   *key = NULL;
    mg_str_t            *arg = cmd->elts;
    
    if(NULL == refers->keys)
    {
        refers->keys = mg_array_create(pool, 8, sizeof(C_dynamic_refer_key_t));
        if(NULL == refers->keys)
        {
            mg_log_ex(MG_LOG_EMERG, "create dynamic refers array failed.");
            return MG_ERROR;
        }
    }

    key = mg_array_push(refers->keys);
    if(NULL == key)
    {
        mg_log_ex(MG_LOG_EMERG, "dynamic refers array push failed.");
        return MG_ERROR;
    }

    key->key = arg[1];

    return MG_OK;
}
