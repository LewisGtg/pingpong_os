// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste de semáforos (light)

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

typedef struct content_t {
   struct content_t * prev;
   struct content_t * next;
   int value;
} content_t;

content_t * content_queue;

task_t p1, p2, c1, c2, c3;
semaphore_t s_buffer, s_item, s_vaga;

// corpo da thread A
void Produtor (void * arg)
{
   int item;

   while (1)
   {
      // printf("passou produtor\n");
      task_sleep(1000);
      item = rand() % 100;

      sem_down(&s_vaga);
      sem_down(&s_buffer);

      content_t * content = malloc(sizeof(content_t));
      content->next = NULL;
      content->prev = NULL;
      content->value = item;
      queue_append((queue_t **) &(content_queue), (queue_t * ) content);
      printf("%s produziu %d (tem %d)\n", (char *) arg, item, queue_size((queue_t *) content_queue));
 
      sem_up(&s_buffer);
      sem_up(&s_item);
   }
   task_exit (0) ;
}

// corpo da thread B
void Consumidor (void * arg)
{
   content_t * content;

   while (1)
   {
      // printf("passou consumidor\n");
      sem_down(&s_item);
      sem_down(&s_buffer);

      content = content_queue;
      if (content)
      {
         queue_remove((queue_t **)&(content_queue), (queue_t *)content);
         printf("                 %s consumiu %d (tem %d)\n", (char *)arg, content->value, queue_size((queue_t *) content_queue));
         free(content);
      }

      sem_up(&s_buffer);
      sem_up(&s_vaga);

      task_sleep(1000);
   }
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf ("main: inicio\n") ;

   ppos_init () ;

   // inicia semaforos
   sem_init (&s_buffer, 1);
   sem_init (&s_item, 0);
   sem_init (&s_vaga, 5);

   // inicia tarefas
   task_init (&p1, Produtor, "p1");
   task_init (&p2, Produtor, "p2");
   task_init (&c1, Consumidor, "c1");
   task_init (&c2, Consumidor, "c2");
   task_init (&c3, Consumidor, "c3");

   task_wait(&p1);

   // destroi semaforos
   sem_destroy (&s_buffer);
   sem_destroy (&s_item);
   sem_destroy (&s_vaga);

   printf ("main: fim\n") ;
   task_exit (0) ;
}