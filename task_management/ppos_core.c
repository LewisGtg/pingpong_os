// GRR20203893 Lewis Guilherme Theophilo Geraldo

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

ucontext_t ContextMain;
task_t TaskMain;
task_t * CurrentTask;

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init ()
{
    if (getcontext(&(TaskMain.context)) == -1)
    {
        fprintf(stderr, "Erro ao inicializar TaskMain!");
        return ;
    }

    TaskMain.id = 0;
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

    makecontext(&(task->context), (void *) start_func, 1, arg);

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
    task_switch(&TaskMain);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task)
{
    task_t *oldTask = CurrentTask;
    CurrentTask = task;

    // (task->context).uc_link = &(oldTask.context);
    swapcontext(&(oldTask->context), &(task->context));
    return 0;
}