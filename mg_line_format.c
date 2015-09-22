#include <stdio.h>
#include "memory_manager.h"
#include "mg_line_format.h"


int mg_get_key_value(mg_str_t *buf, mg_str_t *key, mg_str_t *value_out, mg_str_t *equ_mark, mg_str_t *end_mark)
{
	int 	len;
	int		i;
	int		scanner;
	char	*data;
	mg_str_t	tmp;
	mg_str_t	*value;

	enum{
		S_LOOK = 0,
		S_KEY,
		S_PICK,
		S_OK
	}state;

	if(NULL == buf ||
	   NULL == key ||
		NULL == equ_mark||
		NULL == end_mark)
	{
		return MG_ERROR;
	}

	if(NULL == value_out)
	{
		value = &tmp;
	}
	else
	{
		value = value_out;
	}

	value->len = 0;
	value->data = NULL;

	state = S_LOOK;
	scanner = 0;
	data = buf->data;
	len  = buf->len;
	
	for(i=0; i < len;i++)
	{
		switch(state)
		{
			/*look for user key*/
			case S_LOOK:
				if(data[i] == *(key->data + scanner))
				{
					state = S_KEY;
					scanner ++;
				}
				break;
			case S_KEY:
				if(scanner == key->len)
				{
					scanner = 0;
					if((len - i ) > equ_mark->len &&
					0 == strncmp(equ_mark->data, &data[i],equ_mark->len))
					{
						state = S_PICK;
						i += (equ_mark->len-1);
					}
					else
					if(data[i] == *(key->data + scanner))
					{
						state = S_KEY;
						scanner ++;
					}
					else
					{
						state = S_LOOK;
					}
				}
				else
				if(data[i] != *(key->data + scanner))
				{
					scanner = 0;
					if(data[i] == *(key->data + scanner))
					{
						state == S_KEY;
						scanner ++;
					}
					else
					{
						state = S_LOOK;
					}
				}
				else
				{
					scanner ++;
				}
				break;
			case S_PICK:
				if((len - i ) >= end_mark->len &&
					0 == strncmp(end_mark->data, &data[i],end_mark->len))
				{
					scanner = 0;
					state = S_OK;
					i += (end_mark->len - 1);
				}
				else
				{
					if(NULL == value->data)
					{
						value->data = &data[i];
					}
					value->len ++;
				}
				break;
			case S_OK:
				break;
			
			default:
				return MG_ERROR;
		}

		if(S_OK == state)
		{
			break;
		}
	}

	if(0 == value->data)
	{
		return MG_DECLINED;
	}

	return MG_OK;
}



char * mg_str_to_char(mg_pool_t *pool, mg_str_t * str)
{
	char  *out = NULL;
	int	len;

	mg_log_ex(MG_LOG_INFO,"check.");
	if(NULL == pool|| NULL == str)
	{
		mg_log_ex(MG_LOG_ERROR,"NULL imput.");
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"check OK");

	len = str->len;
	if(0 == len)
	{
		mg_log_ex(MG_LOG_INFO, "string length 0, return empty string.");
		return "";
	}
	
	mg_log_ex(MG_LOG_INFO,"alloc out string.");
	out = mg_pcalloc(pool,sizeof(char)*(len+1));
	if(NULL == out)
	{
		mg_log_ex(MG_LOG_ERROR,"alloc out string error.");
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"alloc out string OK.");
	
	memcpy(out,str->data, len);
	out[len] = '\0';

	mg_log_ex(MG_LOG_INFO,"return out string: %s", out);
	return out;
}


/*查找单词结束位置，结束位置为单词的最后一个字母的下一个位置。设置end_mark为NULL的话，则自动查找单词结尾，下划线及数字不作为分割符。*/
char * mg_find_word_end(mg_pool_t *pool, char * str, char * end_mark)
{
	int	len = 0;
	char	*tmp;
	mg_log_ex(MG_LOG_INFO,"check imput.");
	if(NULL == pool || NULL == str)
	{
		mg_log_ex(MG_LOG_ERROR,"NULL imput.");
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"check imput OK.");

	len = strlen(str);

	if(0 == len)
	{
		mg_log_ex(MG_LOG_INFO,"target string length 0.");
		return NULL;
	}
	
	mg_log_ex(MG_LOG_INFO,"make target string copy.");
	tmp = mg_pcalloc(pool, sizeof(char) * (len+1));
	if(NULL == tmp)
	{
		mg_log_ex(MG_LOG_ERROR,"alloc target string copy error.");
		return NULL;
	}

	memcpy(tmp,str,len);
	tmp[len] = '\0';
	mg_log_ex(MG_LOG_INFO,"target string copy OK.");


	/*search a word*/
	mg_log_ex(MG_LOG_INFO,"looking a word tail.");
	if(NULL == end_mark)
	{
		int	i = 0;
		/*过滤空格*/
		for(i = 0; i < len && (tmp[i] == ' ' || tmp[i] == '\t'); i++);
		
		for(; i < len; i++)
		{
			/*单词合法组成a~z,A~Z,0~9,_ , 遇到其他任意字符认为单词结束*/
			if( 
				 (tmp[i] >= 'a' && tmp[i] <= 'z') ||
				 (tmp[i] >= 'A' && tmp[i] <= 'Z') ||
				 (tmp[i] >= '0' && tmp[i] <= '9') ||
				  tmp[i] == '_'
			     )
			{
				continue;
			}
			
			break;
		}
		
		return str+i;
	}
	/*else*/
	/*end at the end_mark*/
	int	i = 0;
	int	j = 0;
	char	*res = NULL;

	/*过滤空格*/
	for(i = 0; i < len && (tmp[i] == ' ' || tmp[i] == '\t'); i++);

	for(; i < len; i ++)
	{
		/*find end mark, then word found*/
		for(j = 0;end_mark[j] != '\0' && tmp[i+j] == end_mark[j] && i+j < len; j ++);
		if(end_mark[j] == '\0')
		{
			mg_log_ex(MG_LOG_INFO,"word tail found, return");
			return str+i;
		}
		
	}
	
	mg_log_ex(MG_LOG_INFO,"word tail not found, end mark not match.");
	return NULL;
}




/**
* @brief 添加全局变量索引
* @author 马勇
* @date 2012-08-03
*
* @param r
* @param refer		索引词
* @param value		索引值
*
* @return	MG_OK			添加成功
*			MG_ERROR		添加失败	
*/
int mg_add_refer_value(mg_pool_t *pool, mg_array_t *list, mg_str_t *refer, mg_str_t *value)
{
	mg_refer_value_t	**	refer_part = NULL;
	mg_refer_value_t	*	refer_tmp = NULL;

	int		rc;

	/*check imput*/
	mg_log_ex(MG_LOG_INFO,"check imput.");
	if(NULL == pool ||NULL ==  refer || NULL == value)
	{
		mg_log_ex(MG_LOG_ERROR,"NULL imput.");
		return MG_ERROR;
	}
	mg_log_ex(MG_LOG_INFO,"imput check OK.");	

	/*添加索引项，该内存实际保存一指针，未分配实际内存，refer_part类型为 mg_refer_value_t **  */
	mg_log_ex(MG_LOG_INFO,"add value - refer pairs.");
	refer_part = mg_array_push(list);
	if(NULL == refer_part)
	{
		mg_log_ex(MG_LOG_ERROR,"push into list error.");	
		return MG_ERROR;
	}
	/*为索引实际分配内存*/
	refer_tmp = mg_pcalloc(pool,sizeof(mg_refer_value_t));
	if(NULL == refer_tmp)
	{
		mg_log_ex(MG_LOG_ERROR,"alloc refer_tmp memory block error.");	
		return MG_ERROR;
	}
	/*填充索引，mg_str_to_char 会自动创建str的字符串副本*/
	refer_tmp->refer = mg_str_to_char(pool, refer);
	if(NULL == refer_tmp->refer)
	{
		mg_log_ex(MG_LOG_ERROR,"make refer copy error.");	
		return MG_ERROR;
	}

	refer_tmp->value = mg_str_to_char(pool,value);
	if(NULL == refer_tmp->value)
	{
		mg_log_ex(MG_LOG_ERROR,"make value copy error.");	
		return MG_ERROR;
	}

	/*将实际分配空间的地址，赋给新创建的索引项*/
	*refer_part = refer_tmp;
	
	mg_log_ex(MG_LOG_INFO,"refer: %s - value: %s added.", refer_tmp->refer , refer_tmp->value);
	return MG_OK;
}


/*查找list中的 refer value 对， 若查找失败，则返回NULL, list 需要在外部创建*/
char * mg_find_refer_value(mg_pool_t *pool, mg_array_t * list, char * refer)
{
	mg_refer_value_t ** part = NULL;
	char	*p_refer = NULL;
	char  *p_value = NULL;
	int	i = 0;

	mg_log_ex(MG_LOG_INFO,"check imput.");
	if(NULL == pool || NULL == list || NULL == refer)
	{
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"check imput OK.");

	/**if no list part, return*/
	if(list -> nelts <= 0)
	{
		mg_log_ex(MG_LOG_ERROR,"refer - value list empty.");
		return NULL;
	}

	/*scan all list part, try to match the refer str*/
	mg_log_ex(MG_LOG_INFO,"try to match refer name.");
	part = list->elts;

	for(i = 0; i < list->nelts; i++)
	{
		if(0 == strlen(part[i]->refer))
		{
			continue;
		}
		
		if( strlen(refer) == strlen(part[i]->refer) &&
		    0 == strncmp( part[i]->refer, refer, strlen(refer)))
		{
			/*found refer, make a copy and then return*/
			int len = strlen(part[i]->value);
			p_value = mg_pcalloc(pool, sizeof(char)*(len+1));
			if(NULL == p_value)
			{
				return NULL;
			}
			memcpy(p_value, part[i]->value, len);
			p_value[len] = '\0';
			mg_log_ex(MG_LOG_INFO,"matched refer: %s - value: %s.", refer, p_value);
			return p_value;
		}
	} 

	/*not found a refer, return NULL*/
	mg_log_ex(MG_LOG_INFO,"not matched refer: %s.", refer);
	return NULL;
}




/*
	分析格式化串，输出格式化解析以后的数据。
	在格式化串之前，应当自动对list进行更新，避免出现不同步。
	ldap数据应当可以在配置初始化时可以确定，但是处于对数据库
	可能存在的变化考虑，依然需要对其进行更新。
	所有的$refer会被替换为相应的value，默认以单词为分割单位。
	如果希望有字符拼接，可以使用%作为结束符，%不会被输出。
	使用 \+char 可以强制输出任意字符，其中\n \r 会被转义。
	不建议强制输出%,$，避免出现格式化串混乱，有可能造成内存错误。

************************************  
	调整算法，格式化规则不变，对索引内容无任何限制，不存在的索引将
	不做处理，而非返回错误。
*/
char * mg_format_line(mg_pool_t *pool, char *form_in, mg_array_t * list)
{
	/*新算法*/
	char	*	refer  = NULL;			/*用于保存 $value 中的 变量名*/
	char	*	word_start = NULL;		/*单词起始位置*/
	char	*	word_end = NULL;		/*单词结束位置，也会被用在其他段落的起止标记*/
	int	len = 0;						/*各种长度，只要不引起误会，都会重复使用该值*/
	int	i = 0;							/*循环标记*/

	char	*	scanner = NULL;			/*消除 $value 时使用的读取游标*/
	mg_array_t *out_str = NULL;			/*动态数组，用于输出格式化串*/

	char *t_value = NULL;				/*索引值的长度*/
	int  t_len = 0;
	char *nwriter = NULL;				/*写 游标*/


	/*检查输入合法性*/
	mg_log_ex(MG_LOG_INFO,"check imput.");
	if(NULL == pool || NULL == form_in || NULL == list)
	{
		mg_log_ex(MG_LOG_ERROR,"NULL imput.");
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"check imput OK.");

	scanner = form_in;
	/****************创建输出*******************/
	len = strlen(form_in);
	if(len <= 0)
	{
		mg_log_ex(MG_LOG_ERROR,"NULL imput.");
		return "";
	}
	out_str = mg_array_create(pool,2*len, sizeof(char));
	if(NULL == out_str)
	{
		mg_log_ex(MG_LOG_ERROR,"create out put error.");
		return NULL;
	}


	/*这里创建索引list，同时对format中的转义字符进行转义*/
	for(;*scanner != '\0'; scanner ++)
	{
		/*此符号作为扫描终止符号，不输出，如需要输出，使用强制输出符号*/
		if('%' == *scanner)
		{
			continue;
		} 

		/*允许转义 r n*/
		if('\\' == *scanner &&
		   '\0' == *(scanner +1))
		{
		     continue;
		}

		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 'r' == *(scanner+1))
		{
			scanner ++;
			nwriter = mg_array_push(out_str);
			if(NULL == nwriter)
			{
				mg_log_ex(MG_LOG_ERROR,"push into array, error.");
				return NULL;

			}
			*nwriter = '\r';
			continue;
		}

		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 'n' == *(scanner+1))
		{
			scanner ++;
			nwriter = mg_array_push(out_str);
			if(NULL == nwriter)
			{
				mg_log_ex(MG_LOG_ERROR,"push into array, error.");
				return NULL;
			}

			*nwriter = '\n';
			continue;
		}

		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 't' == *(scanner+1))
		{
			scanner ++;
			nwriter = mg_array_push(out_str);
			if(NULL == nwriter)
			{
				mg_log_ex(MG_LOG_INFO,"push into array, error.");
				return NULL;

			}
			*nwriter = '\t';
			continue;
		}
		
		/*强制输出'\'后任意字符*/
		if('\\' == *scanner && 
			'\0' != *(scanner+1))
		{
			scanner ++;
			nwriter = mg_array_push(out_str);
			if(NULL == nwriter)
			{
				mg_log_ex(MG_LOG_ERROR,"push into array, error.");
				return NULL;
			}

			*nwriter = *scanner;
			continue;
		}
		/*发现引用符*/
		if('$' == *scanner)
		{
			/*下一字符开始匹配索引*/
			word_start = scanner+1;
			/*mg_find_word_end会自动忽略空格，故需要测试之*/
			if(' ' == *word_start || '\t' == *word_start)
			{
				continue;
			}
			word_end = mg_find_word_end(pool,word_start,NULL);
			if(NULL == word_end)
			{
				mg_log_ex(MG_LOG_ERROR,"search next word failed.");
				return NULL;
			}
			/*获得长度*/
			len = word_end - word_start;
			if(0 == len)
			{
				mg_log_ex(MG_LOG_ERROR,"next word length 0.");
				continue;
			}
			/*保存变量名索引*/
			refer = mg_pcalloc(pool,sizeof(char)*(len+1));
			if(NULL == refer)
			{
				mg_log_ex(MG_LOG_ERROR,"create refer list part failed.");
				return NULL;
			}
			
			mg_copy(refer,word_start,len);
			refer[len] = '\0';

			/*跳过已解析的变量名部分， -1， 是为了使 scanner＋＋后指向 word_end位置*/
			scanner = word_end-1;
			t_value = mg_find_refer_value(pool,list, refer);
			if(NULL == t_value)
			{
				mg_log_ex(MG_LOG_ERROR,"refer: %s not found!", refer);
				continue;
			}

			t_len = strlen(t_value);
			if(t_len == 0)
			{
				continue;
			}
			nwriter = mg_array_push_n(out_str, t_len);

			memcpy(nwriter, t_value, t_len);

			continue;
		}
		/*其余情况，直接输出*/
		nwriter = mg_array_push(out_str);
		*nwriter = *scanner;
	}

	/*插入结束符*/
	nwriter = mg_array_push(out_str);
	*nwriter = '\0';

	mg_log_ex(MG_LOG_INFO,"format OK.LINE:%s.", out_str->elts);
	return out_str->elts;

}
	#if 0
	char	*	format = NULL;			/*格式化串，其中的格式化符号 $value 会被替换为 %s, 之后 %s会被递归的 消除，生成最终的格式化字符串*/
	
	char	*	refer  = NULL;			/*用于保存 $value 中的 变量名*/
	char	*	word_start = NULL;		/*单词起始位置*/
	char	*	word_end = NULL;		/*单词结束位置，也会被用在其他段落的起止标记*/
	int	len = 0;						/*各种长度，只要不引起误会，都会重复使用该值*/
	int	i = 0;							/*循环标记*/

	mg_array_t	* refer_list = NULL;	/*将format中的 $value 替换为 %s 后， 储存变量名，及其value，便于最终格式化输出*/
	mg_refer_value_t	*refer_part = NULL;	/*用于array遍历*/
	
	char	*	scanner = NULL;			/*消除 $value 时使用的读取游标*/
	char  *  writer = NULL;				/*消除 $value 时使用的写入游标*/

	/*检查输入合法性*/
	mg_log_ex(MG_LOG_INFO,"check imput.");
	if(NULL == pool || NULL == form_in || NULL == list)
	{
		mg_log_ex(MG_LOG_INFO,"NULL imput.");
		return NULL;
	}
	mg_log_ex(MG_LOG_INFO,"check imput OK.");

	mg_log_ex(MG_LOG_INFO,"build $refer list.");
	/*建立变量索引数组，保存格式化字符串中的变量名，其value在后面集中查询处理*/
	refer_list = mg_array_create(pool, 10, sizeof(mg_refer_value_t));
	if(NULL == refer_list)
	{
		mg_log_ex(MG_LOG_ERROR,"create $refer list error.");
		return NULL;
	}

	/*输入串长度，创建输入串副本*/
	len = strlen(form_in);

	/*所有操作均在该副本上执行，创建format str副本*/
	format = mg_pcalloc(pool, sizeof(char)*(len+1));
	memset(format,0,len+1);

	if(NULL == format)
	{
		mg_log_ex(MG_LOG_ERROR,"create format line copy error.");
		return NULL;
	}
	
	/*初始化读写游标，复制输入的格式化串，剔除其中的非法字符，替换 $value 为 %s， 便于后面进行格式化输出*/
	/*这里采用的策略是，
	  1.扫描串中所有的$refer，保存在一个list中。
	  2.查询list中所有refer的值。
	  3.每次将formatstr分解为两段，前一段只有一个$refer, 并替换为%s，以\0结束，第二段为其余
	  数据，利用从本地的索引中查询到的值，对第一段进行格式化，拼合这两段数据，再重复上述操作，直到处理完串
	  中所有的$refer。

	  之所以采用这种低效率的方案，是因为目前无法处理可变参数列表个数不确定的情况，如果有的话，则第三步可以
	  一次将所有的$refer，替换为%s，再依次将所有refer的value作为参数即可。
	*/
	writer = format;
	scanner = form_in;


	/*这里创建索引list，同时对format中的转义字符进行转义*/
	for(;*scanner != '\0'; scanner ++)
	{
		/*form_in 中不允许出现格式化符 %, 直接跳过*/
		if('%' == *scanner)
		{
			continue;
		}
		/*允许转义 r n $*/
		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 'r' == *(scanner+1))
		{
			scanner ++;
			*writer = '\r';
			writer++;
			continue;
		}

		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 'n' == *(scanner+1))
		{
			scanner ++;
			*writer = '\n';
			writer++;
			continue;
		}

		/*允许直接输出 $,  需要在直接输出符号前加斜线 */
		if('\\' == *scanner && 
			'\0' != *(scanner+1) &&
			 '$' == *(scanner+1))
		{
			scanner ++;
			*writer = *scanner;
			writer++;
			continue;
		}
		/*发现引用符*/
		if('$' == *scanner)
		{
			/*下一字符开始匹配索引*/
			word_start = scanner+1;
			word_end = mg_find_word_end(pool,word_start,NULL);
			if(NULL == word_end)
			{
				mg_log_ex(MG_LOG_ERROR,"surch next word failed.");
				return NULL;
			}
			/*获得长度*/
			len = word_end - word_start;
			if(0 == len)
			{
				mg_log_ex(MG_LOG_ERROR,"next word length 0.");
				return NULL;
			}
			/*保存变量名索引*/
			refer = mg_pcalloc(pool,sizeof(char)*(len+1));
			if(NULL == refer)
			{
				mg_log_ex(MG_LOG_ERROR,"create refer list part failed.");
				return NULL;
			}
			
			mg_copy(refer,word_start,len);
			refer[len] = '\0';

			refer_part = mg_array_push(refer_list);
			if(NULL == refer_part)
			{ 
				mg_log_ex(MG_LOG_ERROR,"add refer part into list array error.");
				return NULL;
			}
			refer_part->refer = refer;
			/*跳过已解析的变量名部分， -1， 是为了使 scanner＋＋后指向 word_end位置*/
			scanner = word_end-1;
			/*替换为字符串格式化符  %s，对于合法的输入，这里保证不会造成数据溢出，因为对于变量至少有这样的情况 $V*/
			*writer = '%';
			writer++;
			*writer = 's';
			writer++;
			continue;
		}
		/*其余情况，直接输出*/
		*writer = *scanner;
		writer ++;
	}

	mg_log_ex(MG_LOG_INFO,"build $refer list OK.");

	/*如果没有任何变量，直接输出原串*/
	if(0 == refer_list->nelts)
	{
		mg_log_ex(MG_LOG_INFO,"no refer found, $refer list empty.");
		return format;
	}

	/*变量数*/
	len = refer_list->nelts;
	/*遍历，统一查询所有变量的value*/
	refer_part = refer_list->elts;
	for(i = 0; i < refer_list->nelts; i ++)
	{
		/*查询变量值*/
		refer_part[i].value = mg_find_refer_value(pool,list, refer_part[i].refer);
		if(NULL == refer_part[i].value)
		{
			/*如果存在未注册的变量，将直接返回错误*/
			mg_log_ex(MG_LOG_ERROR,"$%s not exist.", refer_part[i].refer);
			return NULL;
		}
	}

	mg_log_ex(MG_LOG_INFO,"formate line.");
	/*将已获得的 refer value 进行格式化输出，方案如同上述*/
	for(i = 0; i < len; i++)
	{
		/*局部变量*/
		int len = 0;
		int  rc = 0;
		/*该串为format的一部分，从format头部到第一个 %s， 并将对应的 refer value格式化输出到新的format串中，下一个 %s 递归处理*/
		char *in_format = NULL;
		/*每个循环都使用上一次所声称的部分格式化字符串*/
		word_start = format;
		/*获得下一个格式化符所在位置*/
		word_end = mg_find_word_end(pool,word_start,"%s");

		if(NULL == word_end)
		{
			mg_log_ex(MG_LOG_ERROR,"%s not found, cannot format.");
			return NULL;
		}

		/*计算分割格式化串的空间，仅复制第一个%s前的串*/
		len = word_end - word_start + sizeof("%s") -1;
		in_format = mg_pcalloc(pool,sizeof(char)*(len + 1));
		if(NULL == in_format)
		{
			mg_log_ex(MG_LOG_ERROR,"calloc local format buf error.");
			return NULL;
		}
		mg_copy(in_format, word_start, len);
		in_format[len] = '\0';

		/*计算输出第一个格式化符后所需空间*/
		len = snprintf(NULL, 0, in_format, refer_part[i].value);
		len += strlen(format) - (word_end - word_start) ;

		/*以此长度分配空间，新串将减少一个 %s,并被对应的格式化串替代*/
		format = mg_pcalloc(pool,sizeof(char)*(len + 1));
		if(NULL == format)
		{
			mg_log_ex(MG_LOG_ERROR,"calloc now format line buf error.");
			return NULL;
		}
		memset(format,0,len+1);
		/*格式化输出第一个%s*/
		rc = snprintf(format, len, in_format, refer_part[i].value);
		/*将剩余未处理串，串接到尾部，剩余的%s将被下一循环处理*/
		rc += snprintf(format+rc, len - rc,"%s", word_end + sizeof("%s") -1);

		/*内存错误*/
		if(rc > len)
		{
			mg_log_ex(MG_LOG_ERROR,"mem out of range while formating.");
			return NULL;
		}
	}

	mg_log_ex(MG_LOG_INFO,"format OK.LINE:%s.", format);
	return format;
}
	#endif
