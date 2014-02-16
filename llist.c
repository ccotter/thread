
#include <stdlib.h>
#include <llist.h>

void llist_init(llist *list)
{
    list->head = NULL;
    list->size = 0;
}

// Adds an element to the linked list at the beginning. On error, nonzero is returned and the list
// is unchanged. On success, the new element is added to the beginning of the list.
int llist_add_first(llist *list, uint32_t data)
{
    if (NULL == list->head)
    {
        // Allocate memory for head node.
        _llist_node *tmp = (_llist_node*)malloc(sizeof(_llist_node));
        if (NULL == tmp)
        {
            return -E_MEM_ERROR;
        }
        tmp->data = data;
        tmp->next = tmp->prev = tmp;
        list->head = tmp;
        list->size = 1;
        return 0;
    }
    else
    {
        // Allocate memory for new element.
        _llist_node *tmp = (_llist_node*)malloc(sizeof(_llist_node));
        if (NULL == tmp)
        {
            return -E_MEM_ERROR;
        }
        // Change links.
        tmp->data = data;
        tmp->next = list->head;
        tmp->prev = list->head->prev;
        list->head->prev = list->head->prev->next = tmp;
        list->head = tmp; // Set new head node.
        ++list->size;
        return 0;
    }
    return -1; // Should never get here.
}

// Removes the last element from this list and returns the data pointer.
uint32_t llist_remove_last(llist *list)
{
    _llist_node *ptr = list->head->prev;
    uint32_t ret = ptr->data;
    llist_remove(list, ptr);
    return ret;
}

uint32_t llist_remove_first(llist *list)
{
    // Advance the head node one forward, then remove the "last" element.
    list->head = list->head->next;
    return llist_remove_last(list);
}

// Returns a pointer to the first _llist_node whose data pointer is 'data.' Returns NULL
// if no such node exists.
_llist_node *llist_find(llist *list, uint32_t data)
{
    _llist_node *first = list->head;
    _llist_node *at = first;
    // Loop over the list in a sequential manner.
    do
    {
        if (at->data == data)
        {
            return at;
        }
        at = at->next;
    } while (first != at);
    return NULL; // No match.
}

_llist_node *llist_get_first(llist *list)
{
    return list->head;
}

int llist_size(llist *list)
{
    if (NULL == list->head)
        return list->size = 0;
    else
        return list->size;
}

// node must be a valid member of the list.
void llist_remove(llist *list, _llist_node *node)
{
    if (1 == list->size)
    {
        // Removing only member.
        list->size = 0;
        list->head = NULL;
        free(node);
        return;
    }
    else
    {
        // Change links.
        node->prev->next = node->next;
        node->next->prev = node->prev;
        if (list->head == node)
        {
            list->head = node->next;
        }
        free(node);
        --list->size;
    }
}

