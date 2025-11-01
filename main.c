#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Variáveis de tipos e arquivos
const char *logs[]         = { "Interface", "Input", "Operacao", "Localizacao", "Propaganda", "Calculo" };
const char *nomeArquivos[] = { "buffer.log",
                               "Interface.log", "Input.log", "Operacao.log", "Localizacao.log", "Propaganda.log", "Calculo.log",
                               "Omega.log", "KleubsMax.log", "ChirpTome.log"};
#define nTipoLogs 6
#define nArquivos 10

// Funções e structs auxiliares
void moveLogs(const char *origem, const char *destino, const char *palavra);
void geraLogs(const char *name);

typedef struct {
    int id;
    pthread_mutex_t mutex;
} ArquivoData;

typedef struct {
    pthread_t thread;
    bool stop;
    bool acessando[nArquivos]; // true se acessando
} ThreadData;

void LogThread(ThreadData *t);

// Funções das threads
void *ThreadBuffer(void *arg);

// Dados globais
ArquivoData arquivos[nArquivos];
ThreadData threadBuffer;

int main() {
    srand(time(NULL));

    printf("Iniciando...\n");

    // Inicializa os arquivos e mutexes
    for (int i = 0; i < nArquivos; i++) {
        arquivos[i].id = i;
    }

    // Inicializa thread de buffer
    for (int i = 0; i < nArquivos; i++)
        threadBuffer.acessando[i] = false;
    threadBuffer.stop = false;
    pthread_create(&threadBuffer.thread, NULL, ThreadBuffer, &threadBuffer); // arquivo 0 = buffer


    // Deixa rodar por 3 segundos
    sleep(3);

    printf("Parando threads.\n");

    threadBuffer.stop = true;
    pthread_join(threadBuffer.thread, NULL);

    printf("Finalizando.\n");

    return 0;
}

///////////////////////
// Funções das Threads
void *ThreadBuffer(void *arg) {

    ThreadData* myself = (ThreadData*)arg;
    ArquivoData *d = &arquivos[0];

    while(!myself->stop) {
        pthread_mutex_lock(&d->mutex);
        threadBuffer.acessando[0] = true;

        LogThread(myself);
        geraLogs(nomeArquivos[d->id]);

        threadBuffer.acessando[0] = false;
        pthread_mutex_unlock(&d->mutex);

        sleep(1);
    }
    return NULL;
}
///////////////////////


///////////////////////
// Funções auxiliares
void geraLogs(const char *name) {
    FILE *f = fopen(name, "a");

    for (int i = 0; i < 5; i++) {
        fprintf(f, "%s\n", logs[rand() % nTipoLogs]);
    }

    fclose(f);
}
void moveLog(const char *origem, const char *destino, const char *palavra) {
    char tempName[100];
    strcat(tempName, origem);
    strcat(tempName, "_temp.log");

    FILE *fin = fopen(origem, "r");
    FILE *fout = fopen(destino, "a");
    FILE *ftemp = fopen(tempName, "w");

    char linha[256];
    while (fgets(linha, sizeof(linha), fin)) {
        if (strstr(linha, palavra))
            fputs(linha, fout);
        else
            fputs(linha, ftemp);
    }

    fclose(fin);
    fclose(fout);
    fclose(ftemp);


    remove(origem);
    rename(tempName, origem);
}
void LogThread(ThreadData *t) {
    printf("Thread %d:");
    for(int i = 0; i < nArquivos; i++)
        printf(" %c:%d", nomeArquivos[i][0], t->acessando[i]);
    printf("\n");
}
///////////////////////