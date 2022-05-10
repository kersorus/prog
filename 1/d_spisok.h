#ifndef _D_SPISOK_H_
#define _D_SPISOK_H_

#include <stdlib.h>

typedef struct node
{
    void *val;
    struct node *prev;
    struct node *next;
}node_t;

typedef struct list
{
    size_t size;
    node_t ht;
}list_t;

enum LIST_ERRORS
{
    EINV_NODE = -5,
    EINV_FREE = -4,
    EINV_POSN = -3,
    EINV_ALLC = -2,
    EINV_LIST =  1,
    ALLC_ERRR = -1,
    ECALLBACK =  3
};

typedef void *(*allc_t)(size_t, size_t);
typedef void (*free_t)(void *);

node_t *l_find(const list_t *list, size_t posn);  //posn is [1, size]
int     l_create(list_t *list);
int     l_delete__(list_t *list, free_t freer);
int     l_foreach(const list_t *list, int (*callback)(void *data, void *context), void *context);
int     add_before_node__(list_t *list, node_t *node, void *val, allc_t alc);
int     add_after_node__(list_t *list, node_t *node, void *val, allc_t alc);
int     del_node__(list_t *list, node_t *node, free_t freer);

#define l_delete(list)\
        l_delete__(list,free)
#define add_before_node(list,node,val)\
        add_before_node__(list,node,val,calloc)
#define add_after_node(list,node,val)\
        add_after_node__(list,node,val,calloc)
#define del_node(list,node)\
        del_node__(list,node,free)

#endif
