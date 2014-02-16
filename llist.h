
#ifndef _INCLUDE_LLIST_H
#define _INCLUDE_LLIST_H

#include <errors.h>
#include <stdint.h>

struct _llist_node
{
    uint32_t data;
    struct _llist_node *prev;
    struct _llist_node *next;
};
typedef struct _llist_node _llist_node;

// Doubly linked list.
typedef struct
{
    int size;
    _llist_node *head;
} llist;

void llist_init(llist*);

// On success, the first argument is set to point to an initialized linked list head node. Returns 0.
// On failure, != 0 is returned and the first argument is unchanged.
int llist_add_first(llist*, uint32_t);
uint32_t llist_remove_last(llist*);
uint32_t llist_remove_first(llist*);

struct _llist_node *llist_find(llist*, uint32_t);
void llist_remove(llist*, _llist_node*);

_llist_node *llist_get_first(llist*);

int llist_size(llist*);

#endif

