// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

// GRR20203893 Lewis Guilherme Theophilo Geraldo

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

#define STACKSIZE 64*1024

#define PRONTA 0
#define SUSPENSA 1
#define TERMINADA 2

#define MIN_TASK_PRIOR 20
#define MAX_TASK_PRIOR -20
#define AGING 1

#define TICK_RATE 20

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  int exit_code;
  int s_prior ; // prioridade estática
  int d_prior ; // prioridade dinâmica
  int is_kernel_task;
  unsigned int init_time;
  unsigned int exit_time;
  unsigned int execution_time;
  unsigned int processor_time;
  unsigned int activations;
  unsigned int wakeup_time;
  int dependents_qty;
  void * dependents[];
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
