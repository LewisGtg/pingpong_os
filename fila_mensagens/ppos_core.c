// GRR20203893 Lewis Guilherme Theophilo Geraldo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "ppos.h"
#include "queue.h"

task_t DispatcherTask;
task_t TaskMain;
task_t * CurrentTask;
task_t * readyTasksQueue;
task_t * suspenseTasksQueue;

struct sigaction action;
struct itimerval timer;

int lastId = 0;
int readyTasks = 0;
int suspenseTasks = 0;
int tick = TICK_RATE;
int lock_counter = 0 ;
int lock_queue = 0;
int yield_lock = 0;
unsigned int global_time = 0;

void print_elem (void *ptr)
{
   task_t *elem = (task_t *) ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}
 
// leave critical section
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}

void wakeup_tasks()
{
    task_t * aux = suspenseTasksQueue;
    task_t * sleepingTask = aux;

    if (!aux)
        return;

    do
    {
        if (aux->wakeup_time > 0 && aux->wakeup_time <= global_time)
        {
            sleepingTask = aux;
            aux = aux->next;
            sleepingTask->wakeup_time = 0;
            task_resume(sleepingTask, &suspenseTasksQueue);
        }
        else 
            aux = aux->next;
    } while (aux != suspenseTasksQueue && aux->status == SUSPENSA);
}

void resume_dependents(task_t * task)
{
    task_t * aux = task->dep;

    while (queue_size((queue_t *) task->dep) > 0)
    {
        task_resume(aux, &(task->dep));
        aux = aux->next;
    }
}

void task_contabilization(task_t * task)
{
   task->exit_time = systime();
   task->execution_time = task->exit_time - task->init_time;
   printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
          task->id, task->execution_time, task->processor_time, task->activations);
}

task_t * scheduler()
{
    task_t * aux = readyTasksQueue;
    task_t * next = aux;

    if (!aux)
        return NULL;

    // "reseta" a prioridade da ultima tarefa executada
    readyTasksQueue->d_prior = readyTasksQueue->s_prior;

    do
    {
        if (aux->d_prior < next->d_prior)
            next = aux;
        else
        {
            if (aux != next && aux->d_prior > MAX_TASK_PRIOR)
                aux->d_prior -= AGING;
        }

        aux = aux->next;
    }
    while (aux != readyTasksQueue);

    next->activations++;
    readyTasksQueue = next;
    return next;
}

void dispatcher(void * arg)
{
    task_t * next;

    while(readyTasks > 0 || suspenseTasks > 0)
    {
        wakeup_tasks();
        next = scheduler();

        // queue_print("Fila de prontas: ", (queue_t *) readyTasksQueue, print_elem);
        if (next)
        {
            switch (next->status)
            {
            case PRONTA:
                task_switch(next);
                break;

            case TERMINADA:
                break;
            
            default:
                break;
            }
        }
    }

    task_exit(0);
}

void timerHandler()
{
    global_time++;
    CurrentTask->processor_time++;

    if (CurrentTask->is_kernel_task || CurrentTask->status == SUSPENSA || CurrentTask->lock == 1)
        return;

    tick--;

    if (tick <= 0)
    {
        tick = TICK_RATE;
        task_yield();
    }

}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
    if (getcontext(&(TaskMain.context)) == -1)
    {
        fprintf(stderr, "Erro ao iniciar contexto para a tarefa!");
        exit(1);
    }
    queue_append((queue_t **)&readyTasksQueue, (queue_t *)&TaskMain);
    TaskMain.id = lastId++;
    ++readyTasks;

    DispatcherTask.is_kernel_task = 1;
    task_init(&DispatcherTask, dispatcher, NULL);

    CurrentTask = &TaskMain;
    TaskMain.activations++;

    action.sa_handler = timerHandler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro ao iniciar timer do sistema!\n");
        exit(1);
    }

    timer.it_value.tv_usec = 1000;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
      perror("Erro ao iniciar timer do sistema!\n");
      exit (1);
    }

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
    // task_yield();
    return ;
}

// gerência de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) // argumentos para a tarefa
{
    char *stack = malloc(STACKSIZE);

    if (!stack)
    {
        fprintf(stderr, "Erro ao alocar pilha!");
        return -1;
    }

    if (getcontext(&(task->context)) == -1)
    {
        fprintf(stderr, "Erro ao iniciar contexto para a tarefa!");
        return -1;
    }

    (task->context).uc_stack.ss_sp = stack;
    (task->context).uc_stack.ss_size = STACKSIZE;
    (task->context).uc_stack.ss_flags = 0;
    (task->context).uc_link = 0;

    task->next = NULL;
    task->prev = NULL;
    task->id = lastId++;
    task->s_prior = 0;
    task->d_prior = 0;
    // task->is_kernel_task = 0;
    task->status = PRONTA;
    task->init_time = systime();
    task->processor_time = 0;
    task->activations = 0;

    task->dependents_qty = 0;

    makecontext(&(task->context), (void *) start_func, 1, arg);

    // Não é tarefa do SO
    if (task->is_kernel_task != 1)
    {
        queue_append((queue_t **) &readyTasksQueue, (queue_t*) task);
        ++readyTasks;
    }

    return (task->id);
}			

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id ()
{
    return (CurrentTask->id);
}

// Termina a tarefa corrente com um valor de status de encerramento
void task_exit (int exit_code)
{
    if (readyTasks == 0)
    {
        task_contabilization(&DispatcherTask);
        task_switch(&TaskMain);
    }
    else
    {
        readyTasks--;
        CurrentTask->status = TERMINADA;
        CurrentTask->exit_code = exit_code;
        task_contabilization(CurrentTask);
        resume_dependents(CurrentTask);
        queue_remove((queue_t **)&readyTasksQueue, (queue_t *)CurrentTask);
        task_yield();
    }

}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    task_t *oldTask = CurrentTask;
    CurrentTask = task;

    swapcontext(&(oldTask->context), &(task->context));
    return 0;
}

// a tarefa atual libera o processador para outra tarefa
void task_yield ()
{
    CurrentTask->lock = 1;
    DispatcherTask.activations++;
    task_switch(&DispatcherTask);
    CurrentTask->lock = 0;
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio)
{
    if (prio > MIN_TASK_PRIOR)
        prio = MIN_TASK_PRIOR;

    if (prio < MAX_TASK_PRIOR)
        prio = MAX_TASK_PRIOR;

    task->s_prior = prio;
    task->d_prior = prio;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) 
{
    return (task != NULL ? task->s_prior : CurrentTask->s_prior);
}

unsigned int systime ()
{
    return global_time;
}

int task_wait(task_t * task)
{
    if (task == NULL || task->status == TERMINADA)
        return -1;

    task_suspend(&(task->dep));
    task_yield();

    return task->exit_code;
}

void task_suspend (task_t **queue)
{
    CurrentTask->status = SUSPENSA; 
    queue_remove((queue_t **) &readyTasksQueue, (queue_t *) CurrentTask);
    queue_append((queue_t **) queue, (queue_t *) CurrentTask);
    ++suspenseTasks;
    --readyTasks;
}

void task_resume (task_t * task, task_t **queue)
{
    queue_remove((queue_t **) queue, (queue_t *) task);
    queue_append((queue_t **) &readyTasksQueue, (queue_t *) task);
    task->status = PRONTA;
    --suspenseTasks;
    ++readyTasks;
}

void task_sleep(int t)
{
    CurrentTask->wakeup_time = global_time + t;
    task_suspend(&suspenseTasksQueue);
    task_yield();
}

int sem_init (semaphore_t *s, int value)
{
   s->counter = value;
   s->semaphore_ended = 0;
//    s->semaphore_queue = NULL; 

   return 0;
}

int sem_down(semaphore_t *s)
{
    if (!s || s->semaphore_ended)
        return -1;       

    enter_cs(&lock_counter);
   --s->counter;
   leave_cs(&lock_counter);

   if (s->counter < 0)
   {
        enter_cs(&lock_queue);
        task_suspend(&(s->semaphore_queue));
        leave_cs(&lock_queue);
        task_yield();
   }

   return 0;
}

int sem_up(semaphore_t *s)
{
    if (!s || s->semaphore_ended)
        return -1;

    task_t *aux;
    enter_cs(&lock_counter);
    ++s->counter;
    leave_cs(&lock_counter);

    if (s->counter <= 0)
    {
        enter_cs(&lock_queue);
        aux = s->semaphore_queue;
        task_resume(aux, &(s->semaphore_queue));
        leave_cs(&lock_queue);
    }

    return 0;
}

int sem_destroy(semaphore_t *s)
{
   if (!s || s->semaphore_ended)
        return -1;
    
    task_t * aux = s->semaphore_queue;

   while (queue_size((queue_t *)s->semaphore_queue) > 0)
   {
        task_resume(aux, &(s->semaphore_queue));
        aux = s->semaphore_queue;
   }    

    s->semaphore_ended = 1;
   s = NULL;
   return 0;
}

int mqueue_init(mqueue_t *queue, int max, int size)
{
   sem_init(&(queue->s_buffer), 1);
   sem_init(&(queue->s_item), 0);
   sem_init(&(queue->s_vaga), max);

   queue->msg_size = size;
   return 0;
}

int mqueue_send(mqueue_t *queue, void *msg)
{
   if (!queue)
        return -1;

   if (sem_down(&(queue->s_vaga)) < 0) return -1;
   sem_down(&(queue->s_buffer));

   mcontent_t *content = malloc(sizeof(mcontent_t));
   content->value = malloc(queue->msg_size);
   memcpy(content->value, msg, queue->msg_size);

   content->next = NULL;
   content->prev = NULL;

   queue_append((queue_t **)&(queue->buffer), (queue_t *)content);

   sem_up(&(queue->s_buffer));
   sem_up(&(queue->s_item));

   return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg)
{
   if (!queue)
        return -1;

   sem_down(&(queue->s_item));
   sem_down(&(queue->s_buffer));

   mcontent_t *content = queue->buffer;
   if (!content)
        return -1;

   memcpy(msg, content->value, queue->msg_size);
   queue_remove((queue_t **)&(queue->buffer), (queue_t *)content);
   free(content->value);
   free(content);

   sem_up(&(queue->s_buffer));
   sem_up(&(queue->s_vaga));

   return 0;
}

int mqueue_destroy(mqueue_t *queue)
{
    if (!queue)
        return -1;

    sem_destroy(&(queue->s_buffer));
    sem_destroy(&(queue->s_item));
    sem_destroy(&(queue->s_vaga));

    mcontent_t *aux = queue->buffer;

    while (aux != NULL)
    {
        queue_remove((queue_t **)&(queue->buffer), (queue_t *)aux);
        free(aux);
        aux = queue->buffer;
    }

    queue = NULL;
    return 0;
}

int mqueue_msgs(mqueue_t *queue)
{
    if (!queue)
        return -1;

    return queue_size((queue_t *)queue);
}