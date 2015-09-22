#ifndef MG_STRING_H
#define MG_STRING_H

#include <string.h>
#include "mg_type.h"

typedef struct {
    size_t      len;
    u_char     *data;
} mg_str_t;



#define mg_string(str)     { sizeof(str) - 1, (u_char *) str }
#define mg_null_string     { 0, NULL }
#define mg_str_set(str, text)                                               \
    (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text
#define mg_str_null(str)   (str)->len = 0; (str)->data = NULL

#define mg_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define mg_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))

#define mg_str_eq_char(str, chars) (strlen(chars) == (str)->len && 0 == strncmp((chars), (str)->data, (str)->len))
#define mg_str_eq_str(stra, strb)  ((stra)->len == (strb)->len && 0 == strncmp((stra)->data, (strb)->data, (stra)->len))
#define mg_char_eq_char(chara, charb) 	(strlen(chara) == strlen(charb) && 0 == strncmp((chara), (charb), strlen(chara)))

static inline u_char *
mg_copy(u_char *dst, u_char *src, size_t len)
{
    if (len < 17) {

        while (len) {
            *dst++ = *src++;
            len--;
        }

        return dst;

    } else {
        return mg_cpymem(dst, src, len);
    }
}


#endif
