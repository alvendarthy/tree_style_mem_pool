#ifndef MG_LINE_FORMAT_H
#define MG_LINE_FORMAT_H

typedef struct 
{
	char	* refer;
	char	* value;
}mg_refer_value_t;


typedef struct
{
	mg_str_t	left;
	mg_str_t	right;
}mg_str_pair_t;


char * mg_str_to_char(mg_pool_t *pool, mg_str_t * str);

int mg_get_key_value(mg_str_t *buf, mg_str_t *key, mg_str_t *value_out, mg_str_t *equ_mark, mg_str_t *end_mark);

#define mg_refer_list_create(pool, n) (mg_array_create((pool),(n), sizeof(mg_refer_value_t*)))

/*查找单词结束位置，结束位置为单词的最后一个单词的下一个位置。设置end_mark为NULL的话，则自动查找单词结尾，下划线及数字不作为分割符。*/
char * mg_find_word_end(mg_pool_t *pool, char * str, char * end_mark);

int mg_add_refer_value(mg_pool_t *pool, mg_array_t *list, mg_str_t *refer, mg_str_t *value);

char * mg_find_refer_value(mg_pool_t *pool, mg_array_t * list, char * refer);


/*
	分析格式化串，输出格式化解析以后的数据。
	在格式化串之前，应当自动对list进行更新，避免出现错误。
	ldap数据应当可以在配置初始化时可以确定，但是处于对数据库
	可能存在的变化考虑，依然需要对其进行更新。
*/
char * mg_format_line(mg_pool_t *pool, char *form_in, mg_array_t * list);

#endif
