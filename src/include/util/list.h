#ifndef LIST_IMPL

#define LIST_IMPL

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct LinkedList {
    void* value;
    void* next;
} LinkedList;

int LinkedList_length(LinkedList list) {
    int len = 0;
    LinkedList currentItem = list;
    
    while (true) {
        if (currentItem.next == NULL) {
            return len;
        }
        
        len++;
    }
}

LinkedList LinkedList_atIndex(LinkedList list, int index) {
    LinkedList currentItem = list;

    if (index < 0) {
        index = LinkedList_length(list) + index + 1; // -1 = length(list), -2 = length(list) - 1, etc..
    }
    
    while (index != 0) {
        if (currentItem.next == NULL) {
            perror("Indexing out of range");
            _exit(1);
        }

        currentItem = *((LinkedList*)(currentItem.next));
        index--;
    }

    return currentItem;
}

void LinkedList_push(LinkedList list, void* value) {
    LinkedList lastItem = LinkedList_atIndex(list, -1);
    
    LinkedList* new_ptr = malloc(sizeof(LinkedList));
    LinkedList new = *new_ptr;

    new.next = NULL;
    new.value = value;

    lastItem.next = new_ptr;
}

#endif

#ifndef NULL
#define NULL ((void*)0)
#endif