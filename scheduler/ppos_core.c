// GRR20203893 Lewis Guilherme Theophilo Geraldo

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

task_t DispatcherTask;
task_t TaskMain;
task_t * CurrentTask;
task_t * userTasksQueue;

int lastId = 0;
int userTasks = 0;

void print_elem (void *ptr)
{
   task_t *elem = (task_t *) ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
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
            task_switch(next);

            switch (next->status)
            {
            case PRONTA:
                /* code */
                break;

            case TERMINADA:
                queue_remove((queue_t **)&userTasksQueue, (queue_t *) next);
                userTasks--;
                break;
            
            default:
                break;
            }
        }
    }

    task_exit(0);
}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
    task_init(&TaskMain, NULL, NULL);
    task_init(&DispatcherTask, dispatcher, NULL);
    CurrentTask = &TaskMain;

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
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

    makecontext(&(task->context), (void *) start_func, 1, arg);

    // Não é tarefa do SO
    if (task != &DispatcherTask)
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
        task_switch(&TaskMain);
    else
    {
        CurrentTask->status = TERMINADA;
        task_switch(&DispatcherTask);
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