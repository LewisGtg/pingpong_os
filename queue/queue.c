// GRR20203893 Lewis Guilherme
#include "queue.h"

int queue_size (queue_t *queue) {
    queue_t * queueItem = queue->next;
    queue_t * firstItem = queue->next;
    int queueSize = queueItem != NULL ? 1 : 0;

    while (queueItem != NULL && queueItem->next != firstItem)
        ++queueSize;

    return queueSize;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    return ;
}

int queue_append (queue_t **queue, queue_t *elem) {
    queue_t * queueHead = *queue;
    queue_t * queueItem = queueHead->next;

    // Lista está vazia
    if (!queueItem) {
        elem->next = elem;
        elem->prev = elem;
        queueHead->next = elem;
    }
    else {
        queue_t * aux = queueItem;

        while (aux->next != queueItem) {
            if (aux == elem)
                return 1;

            aux = aux->next;
        }

        elem->next = queueItem;
        elem->prev = aux->prev;
        aux->prev->next = elem;
        queueItem->prev = elem; 
    }

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    return 0;
}