#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// --- Constantes Globais ---
#define N_TIPO_LOGS 6
#define N_ARQUIVOS 10
#define N_THREADS_ORGANIZADORAS 5
#define N_THREADS_EMPRESAS 3
#define N_THREADS_TOTAL (1 + N_THREADS_ORGANIZADORAS + N_THREADS_EMPRESAS)

// Índices para os arquivos
#define IDX_BUFFER 0
#define IDX_INTERFACE 1
#define IDX_INPUT 2
#define IDX_OPERACAO 3
#define IDX_LOCALIZACAO 4
#define IDX_PROPAGANDA 5
#define IDX_CALCULO 6
#define IDX_OMEGA 7
#define IDX_KLEUBSMAX 8
#define IDX_CHIRPTOME 9

extern const char *logs[];
extern const char *nomeArquivos[];
extern const char *threadNames[N_THREADS_TOTAL];

// --- Estruturas de Dados ---

// Representação de arquivo com mutex para travar thread
typedef struct {
    int id;
    pthread_mutex_t mutex;
    int accessing_thread_id; // Id da thread que está segurando o arquivo no momento
} ArquivoData;

// Representação de uma Thread, com flag de acesso e pausa
typedef struct {
    pthread_t thread;
    int id_logico; // Para sabermos qual thread é (0-8)
    bool stop;
    bool acessando[N_ARQUIVOS];
    int target_arquivo_id; // Id do arquivo alvo que a thread quer travar e acessar
} ThreadData;

// Representação dos argumentos para gerenciamento das threads de logs (organizadoras)
typedef struct {
    ThreadData *data;            // Ponteiro para seus próprios dados de thread
    const char *log_type;        // O tipo de log que ela procura (ex: "Interface")
    int idx_arquivo_origem;      // Sempre IDX_BUFFER
    int idx_arquivo_destino;     // O arquivo de empresa para onde ela deve mover (ex: IDX_OMEGA)
} ThreadOrganizadoraArgs;

// Representação dos argumentos para gerenciamento das threads de empresas
typedef struct {
    ThreadData *data;                                // Ponteiro para seus próprios dados de thread
    const char **log_types;                         // O tipo de logs que a empresa procura, sendo sempre 3
    int* lista_idx_arquivo_origem;                   // IDs dos logs que a empresa procura, sendo sempre 3
    int idx_arquivo_destino;                         // O arquivo para onde ela deve mover (ex: IDX_OMEGA)
    int num_fontes;                                 // Tamanho do array de fontes
} ThreadEmpresaArgs;

// --- Dados Globais (Declarações) ---
extern ArquivoData arquivos[N_ARQUIVOS];

// --- Funções Auxiliares ---
void moveLogs(const char *origem, const char *destino, const char *palavra);
void geraLogs(const char *name);
void LogThread(ThreadData *t);
void liberarTodosLocks(ThreadData *myself); // Função para thread liberar todos seus locks de mutexes

// --- Funções das Threads ---
void *ThreadBuffer(void *arg);
void *ThreadOrganizadora(void *arg);
void *ThreadEmpresa(void *arg);

#endif
