// GRR20203893 Lewis Guilherme
#include "queue.h"
#include <stdio.h>

int queue_size (queue_t *queue) {
    if (!queue)
        return 0;
    
    queue_t *aux = queue;
    int size = 1;

    while (aux->next != queue) {
        aux = aux->next;
        ++size;
    }

    return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    printf("%s:", name);

    if (!queue) {
        printf(" []\n");
        return ;
    }

    queue_t *aux = queue;

    do
    {
        if (aux == queue)
            printf(" [");

        print_elem(aux);

        if (aux->next == queue)
            printf("]\n");
        else
            printf(" ");

        aux = aux->next;
    } while (aux != queue);

    return ;
}

int queue_append (queue_t **queue, queue_t *elem) {
    if (elem->next || elem->prev) {
        fprintf(stderr, "Erro: Elemento já pertence à uma fila!\n");
        return -1;
    }

    // Queue está vazia, apenas aponta para o elemento
    if (!(*queue)) {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return 0;
    }

    // Aponta para o último elemento da queue
    queue_t *aux = (*queue)->prev;

    // Insere na última posição
    elem->next = *queue;
    elem->prev = aux;
    aux->next = elem;
    (*queue)->prev = elem;

    return 0;
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    // Queue está vazia, não tem oque remover
    if (!(*queue)) {
        fprintf(stderr, "Erro: A fila não existe!\n");
        return -1;

    }

    queue_t *aux = *queue;

    // Deseja remover o primeiro elemento
    if (elem == *queue) {
        // Há apenas um elemento na queue
        if ((*queue)->next == *queue)
            *queue = NULL;
        else
        {
            (*queue)->next->prev = (*queue)->prev;
            (*queue)->prev->next = (*queue)->next;
            *queue = (*queue)->next;
        }

        aux->next = NULL;
        aux->prev = NULL;

        return 0;
    }

    do 
    {
        if (aux == elem)
        {
            aux->prev->next = aux->next;
            aux->next->prev = aux->prev;
            aux->next = NULL;
            aux->prev = NULL;
            return 0;
        }
        aux = aux->next;
    } while (aux != *queue);

    fprintf(stderr, "Erro: Elemento não pertence à fila!\n");
    return -1;
}