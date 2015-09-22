#ifndef MG_QUEUE_H
#define MG_QUEUE_H

#include <stddef.h>

typedef struct mg_queue_s  mg_queue_t;

struct mg_queue_s 
{
    mg_queue_t  *prev;
    mg_queue_t  *next;
};

#define mg_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q


#define mg_queue_empty(h)                                                    \
    (h == (h)->prev)


#define mg_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define mg_queue_insert_after   mg_queue_insert_head


#define mg_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x

#define mg_queue_insert_before  mg_queue_insert_tail

#define mg_queue_head(h)                                                     \
    (h)->next.

#define mg_queue_last(h)                                                     \
    (h)->prev


#define mg_queue_sentinel(h)                                                 \
    (h)


#define mg_queue_next(q)                                                     \
    (q)->next


#define mg_queue_prev(q)                                                     \
    (q)->prev

#define mg_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#define mg_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))


#endif
