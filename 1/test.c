#include "d_spisok.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CHECKR(condition)\
        if (condition)\
        {\
            fprintf(stderr, "TEST FAILED: %s():%d:\n\t%s\n", __PRETTY_FUNCTION__, __LINE__, #condition);\
            return 1;\
        }\

#define BAD_CONTEXT -33

void *b_calloc(size_t nmemb, size_t size)
{
    size_t res = nmemb + size;
    res++;

    return (void *) 0;
}

int for_foreach(void *data, void *context)
{
    if (context == (void *) -1)
        return BAD_CONTEXT;

    *((int *) context) += (uint64_t) data;

    return 0;
}

int main()
{

    list_t main_list;
    list_t *list = &main_list;
    uint64_t val[6] = {0, 1, 2, 3, 4, 5};
    node_t *nodes[6];

//------------------------------------------------------------------------------

// l_create()           TEST

    int res = l_create(NULL);
    CHECKR(res != EINV_LIST)
    l_create(list);
    CHECKR(list->size != 0)
    CHECKR(list->ht.next != &(list->ht))
    CHECKR(list->ht.prev != &(list->ht))

//------------------------------------------------------------------------------

// allocation TESTS

 //add_before_node

    CHECKR(add_before_node__(list, &(list->ht), (void *) val[0], b_calloc) != ALLC_ERRR)

 //add_after_node

    CHECKR(add_after_node__(list, &(list->ht), (void *) val[0], b_calloc) != ALLC_ERRR)

//------------------------------------------------------------------------------

//add_after_node        TEST

    CHECKR(add_after_node(NULL, &(list->ht), (void *) val[0]) != EINV_LIST)
    CHECKR(add_after_node(list, NULL, (void *) val[0]) != EINV_NODE)
    CHECKR(add_after_node__(list, &(list->ht), (void *) val[0], NULL) != EINV_ALLC)
    add_after_node(list, &(list->ht), (void *) val[0]);          // [0]
    CHECKR(list->ht.next->val != (void *) val[0])
    CHECKR(list->ht.prev->val != (void *) val[0])
    CHECKR(list->size != 1)

//l_find                TEST

    CHECKR(l_find(NULL, 1) != (node_t *) EINV_LIST)
    CHECKR(l_find(list, -1) != (node_t *) EINV_POSN)
    nodes[0] = l_find(list, 1);
    CHECKR(nodes[0] != list->ht.next)

//add_after_node cntn   TEST

    add_after_node(list, nodes[0], (void *) val[1]);         // [0, 1]
    nodes[1] = l_find(list, 2);
    CHECKR(nodes[1]->val != (void *) val[1])
    CHECKR(nodes[1]->next != &(list->ht))
    CHECKR(nodes[1]->prev != nodes[0])
    add_after_node(list, nodes[0], (void *) val[2]);        // [0, 2, 1]
    nodes[2] = l_find(list, 2);
    CHECKR(nodes[2]->val != (void *) val[2])
    CHECKR(nodes[2]->prev != nodes[0])
    CHECKR(nodes[2]->next != nodes[1])

//add_before_node       TEST

    CHECKR(add_before_node(NULL, &(list->ht), (void *) val[0])!= EINV_LIST)
    CHECKR(add_before_node(list, NULL, (void *) val[0]) != EINV_NODE)
    CHECKR(add_before_node__(list, &(list->ht), (void *) val[0], NULL) != EINV_ALLC)
    add_before_node(list, nodes[0], (void *) val[3]);            // [3, 0, 2, 1]
    nodes[3] = l_find(list, 1);
    CHECKR(nodes[3]->val != (void *) val[3])
    CHECKR(nodes[3]->next != nodes[0])
    CHECKR(nodes[3]->prev != &(list->ht))
    add_before_node(list, nodes[0], (void *) val[4]);            // [3, 4, 0, 2, 1]
    nodes[4] = l_find(list, 2);
    CHECKR(nodes[4]->val != (void *) val[4])
    CHECKR(nodes[4]->next != nodes[0])
    CHECKR(nodes[4]->prev != nodes[3])
    add_before_node(list, &(list->ht), (void *) val[5]);            // [3, 4, 0, 2, 1, 5]
    nodes[5] = l_find(list, 6);
    CHECKR(nodes[5]->val != (void *) val[5])
    CHECKR(nodes[5]->prev != nodes[1])
    CHECKR(nodes[5]->next != &(list->ht))

//l_foreach             TEST

    uint64_t cont = 0;
    CHECKR(l_foreach(NULL, for_foreach, (void *) cont) != EINV_LIST)
    CHECKR(l_foreach(list, NULL, (void *) cont) != ECALLBACK)
    CHECKR(l_foreach(list, for_foreach, (void *) -1) != BAD_CONTEXT)
    l_foreach(list, for_foreach, (void *) &cont);
    CHECKR(cont != 15)

//del_node              TEST

    CHECKR(del_node(NULL, nodes[3]) != EINV_LIST)
    CHECKR(del_node(list, NULL) != EINV_NODE)
    CHECKR(del_node__(list, nodes[3], NULL) != EINV_FREE)
    del_node(list, nodes[3]);            // [4, 0, 2, 1, 5]
    CHECKR(list->ht.next != nodes[4])
    CHECKR(nodes[4]->prev != &(list->ht))
    del_node(list, nodes[0]);            // [4, 2, 1, 5]
    CHECKR(nodes[4]->next != nodes[2])
    CHECKR(nodes[2]->prev != nodes[4])
    del_node(list, nodes[5]);            // [4, 2, 1]
    CHECKR(nodes[1]->next != &(list->ht))
    CHECKR(list->ht.prev != nodes[1])

//l_delete              TEST

    CHECKR(l_delete(NULL) != EINV_LIST)
    CHECKR(l_delete__(list, NULL) != EINV_FREE)
    l_delete(list);

//------------------------------------------------------------------------------

    fprintf(stderr, "All tests succeeded.\n");

    return 0;
}
