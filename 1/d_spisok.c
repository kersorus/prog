#include "d_spisok.h"

node_t *l_find(const list_t *list, size_t posn)
{
    if (!list)
        return (node_t *) EINV_LIST;

    if ((posn > list->size) || (posn == 0))
        return (node_t *) EINV_POSN;

    node_t *node = list->ht.next;
    for(size_t ind = 1; (ind < posn) && node; ind++)
        node = node->next;

    return node;
}

int l_create(list_t *list)
{
    if (!list)
        return EINV_LIST;

    list->size = 0;
    list->ht.next = &(list->ht);
    list->ht.prev = &(list->ht);

    return 0;
}

int l_delete__(list_t *list, free_t freer)
{
    if (!list)
        return EINV_LIST;

    if (!freer)
        return EINV_FREE;

    node_t *node = list->ht.next;
    for (size_t ind = 0; ind < list->size; ind++)
    {
        node_t *to_delete = node;
        node = node->next;
        freer(to_delete);
    }

    return 0;
}

int l_foreach(const list_t *list, int (*callback)(void *data, void *context), void *context)
{
    if (!list)
        return EINV_LIST;

    if (!callback)
        return ECALLBACK;

    node_t *node = list->ht.next;
    for (size_t ind = 1; ind <= list->size; ind++)
    {
        int res = callback((void *) node->val, context);

        if (res)
            return res;

        node = node->next;
    }

    return 0;
}

int add_before_node__(list_t *list, node_t *node, void *val, allc_t allc)
{
    if (!list)
        return EINV_LIST;

    if (!node)
        return EINV_NODE;

    if (!allc)
        return EINV_ALLC;

    node_t *new_node = (node_t *) allc(1, sizeof(node_t));
    if (!new_node)
        return ALLC_ERRR;

    new_node->val = val;
    new_node->prev = node->prev;
    new_node->next = node;

    node->prev->next = new_node;
    node->prev = new_node;

    list->size++;

    return 0;
}

int add_after_node__(list_t *list, node_t *node, void *val, allc_t allc)
{
    if (!list)
        return EINV_LIST;

    if (!node)
        return EINV_NODE;

    if (!allc)
        return EINV_ALLC;

    node_t *new_node = (node_t *) allc(1, sizeof(node_t));
    if (!new_node)
        return ALLC_ERRR;

    new_node->val = val;
    new_node->prev = node;
    new_node->next = node->next;

    node->next->prev = new_node;
    node->next = new_node;

    list->size++;

    return 0;
}

int del_node__(list_t *list, node_t *node, free_t freer)
{
    if (!list)
        return EINV_LIST;

    if (!node || (node == &(list->ht)))
        return EINV_NODE;

    if (!freer)
        return EINV_FREE;

    node->next->prev = node->prev;
    node->prev->next = node->next;

    freer(node);

    list->size--;

    return 0;
}
