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

// --- Estruturas de Dados ---

// Representação de arquivo com mutex para travar thread
typedef struct {
    int id;
    pthread_mutex_t mutex;
} ArquivoData;

// Representação de uma Thread, com flag de acesso e pausa
typedef struct {
    pthread_t thread;
    int id_logico; // Para sabermos qual thread é (0-8)
    bool stop;
    bool acessando[N_ARQUIVOS];
} ThreadData;

// --- Dados Globais (Declarações) ---
extern ArquivoData arquivos[N_ARQUIVOS];

// --- Funções Auxiliares ---
void moveLogs(const char *origem, const char *destino, const char *palavra);
void geraLogs(const char *name);
void LogThread(ThreadData *t);

// --- Funções das Threads ---
void *ThreadBuffer(void *arg);
void *ThreadOrganizadora(void *arg);
void *ThreadEmpresa(void *arg);

#endif
