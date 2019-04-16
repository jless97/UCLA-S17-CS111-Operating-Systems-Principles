////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less
// EMAIL: jaless1997@gmail.com
// ID: 404-640-158
// Project 2a: SortedList.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define _GNU_SOURCE

#include "SortedList.h"
#include <pthread.h>
#include <sched.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    // If the list is empty, or the element to insert isn't valid, then instantly return
    if (list == NULL || element == NULL) {
        return;
    }
    // If list isnt empty, then begin search for position to insert element at the head
    SortedListElement_t *curr = list->next;
    // While there are still valid elements left in the list, continue search
    while (curr != list) {
        // As the list is sorted, find where element fits in list, if found, then insert
        if (strcmp(element->key, curr->key) <= 0)
            break;
        curr = curr->next;
    }
    
    // Critical section for insert is when the list is being update to relink with new element
    if (opt_yield & INSERT_YIELD)
        sched_yield();
    
    // Properly relink the prev and curr nodes to point to the new element and vice versa
    element->next = curr;
    element->prev = curr->prev;
    curr->prev->next = element;
    curr->prev = element;
}

int
SortedList_delete( SortedListElement_t *element) {
    if (element == NULL) {
        return 1;
    }
    // Check to make sure that the prev/next pointers aren't corrupted
    if (element->next->prev == element->prev->next) {
        // Critical section for delete is when updating the list to remove the given element
        if (opt_yield & DELETE_YIELD)
            sched_yield();
        
        // Prev/next pointers aren't corrupted, so delete element from the list
        element->next->prev = element->prev;
        element->prev->next = element->next;
        return 0;
    }
    return 1;
}

SortedListElement_t*
SortedList_lookup(SortedList_t *list, const char *key) {
    // If the list is empty, or the desired key is invalid
    if (list == NULL || key == NULL) {
        return NULL;
    }
    // If list isnt empty, then begin search for desired key at the head
    SortedListElement_t *curr = list->next;
    // While there are still valid elements left in the list, continue search
    while (curr != list) {
        // If the current node's key is equal to desired key, we found our node, so break
        if (strcmp(curr->key, key) == 0)
            return curr;
        
        // Critical section for lookup involves updating search to next node
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        curr = curr->next;
    }
    
    return NULL;
}

int
SortedList_length(SortedList_t *list) {
    // If list is corrupted
    if (list == NULL) {
        return -1;
    }

    int list_length = 0;
    SortedListElement_t *curr = list->next;
    // While there are still valid elements in the list, continue counting
    while (curr != list) {
        list_length++;
        
        // Critical section for length is when the count is between when count is being updated,
        // and the node is updated
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        
        curr = curr->next;
    }
    
    return list_length;
}
