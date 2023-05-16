// GRR20203893 Lewis Guilherme Theophilo Geraldo

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include "ppos.h"
#include "queue.h"

task_t DispatcherTask;
task_t TaskMain;
task_t * CurrentTask;
task_t * userTasksQueue;

struct sigaction action;
struct itimerval timer;

int lastId = 0;
int userTasks = 0;
int tick = TICK_RATE;
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

void task_contabilization(task_t * task)
{
   task->exit_time = systime();
   task->execution_time = task->exit_time - task->init_time;
   printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
          task->id, task->execution_time, task->processor_time, task->activations);
}

task_t * scheduler()
{
    task_t * aux = userTasksQueue;
    task_t * next = aux;

    // "reseta" a prioridade da ultima tarefa executada
    userTasksQueue->d_prior = userTasksQueue->s_prior;

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
    while (aux != userTasksQueue);

    next->activations++;
    userTasksQueue = next;
    return next;
}

void dispatcher(void * arg)
{
    task_t * next;

    while(userTasks > 0)
    {
        next = scheduler();

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
    if (CurrentTask->is_kernel_task)
        return;

    ++global_time;
    ++CurrentTask->processor_time;
    --tick;

    if (tick == 0)
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
    queue_append((queue_t **)&userTasksQueue, (queue_t *)&TaskMain);
    TaskMain.id = lastId++;
    ++userTasks;

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

    makecontext(&(task->context), (void *) start_func, 1, arg);

    // Não é tarefa do SO
    if (task->is_kernel_task != 1)
    {
        queue_append((queue_t **) &userTasksQueue, (queue_t*) task);
        ++userTasks;
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
    if (userTasks == 0)
    {
        task_contabilization(&DispatcherTask);
        task_switch(&TaskMain);
    }
    else
    {
        userTasks--;
        CurrentTask->status = TERMINADA;
        task_contabilization(CurrentTask);
        queue_remove((queue_t **)&userTasksQueue, (queue_t *)CurrentTask);
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
    DispatcherTask.activations++;
    task_switch(&DispatcherTask);
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