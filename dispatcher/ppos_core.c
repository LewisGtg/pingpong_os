// GRR20203893 Lewis Guilherme Theophilo Geraldo

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

task_t DispatcherTask;
task_t * CurrentTask;
task_t * userTasksQueue;

int lastId = 0;
int userTasks = 0;

task_t * scheduler()
{
    return (CurrentTask == &DispatcherTask ? userTasksQueue : CurrentTask->next);
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
                --userTasks;
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
    task_init(&DispatcherTask, dispatcher, NULL);
    CurrentTask = &DispatcherTask;

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

    makecontext(&(task->context), (void *) start_func, 1, arg);

    // Não é tarefa do SO
    if (task->id != 0)
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
    CurrentTask->status = TERMINADA;
    task_switch(&DispatcherTask);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    task_t *oldTask = CurrentTask;
    CurrentTask = task;

    // (task->context).uc_link = &(oldTask.context);
    if (&(oldTask->context) == &(task->context))
        setcontext(&(task->context));
    else
        swapcontext(&(oldTask->context), &(task->context));
    return 0;
}

// a tarefa atual libera o processador para outra tarefa
void task_yield ()
{
    task_switch(&DispatcherTask);
}
