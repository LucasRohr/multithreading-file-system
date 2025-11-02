#include "common.h"

// Arquivos globais
ArquivoData arquivos[N_ARQUIVOS];

int main() {
    srand(time(NULL));
    printf("Iniciando...\n");

    // 1. Inicializa os arquivos e mutexes
    for (int i = 0; i < N_ARQUIVOS; i++) {
        arquivos[i].id = i;

        if (pthread_mutex_init(&arquivos[i].mutex, NULL) != 0) {
            perror("Falha ao inicializar mutex");
            exit(EXIT_FAILURE);
        }
    }

    // 2. Prepara dados e cria as threads
    ThreadData threadBuffer;
    ThreadData threadsOrg[N_THREADS_ORGANIZADORAS];
    ThreadData threadsEmp[N_THREADS_EMPRESAS];

    pthread_create(&threadBuffer.thread, NULL, ThreadBuffer, &threadBuffer); // Cria thread do buffer

    // Inicializar e criar as 5 Threads Organizadoras
    // (Passar argumentos para elas saberem o que fazer)
    // for (i = 0; i < N_THREADS_ORGANIZADORAS; i++) { ... }

    // Inicializar e criar as 3 Threads das Empresas
    // (Também precisarão de argumentos)
    // for (i = 0; i < N_THREADS_EMPRESAS; i++) { ... }

    // 3. Loop principal
    // Por enquanto, apenas um sleep
    sleep(10);

    // 4. Sinalizar parada para todas as threads
    printf("Parando threads.\n");

    threadBuffer.stop = true;
    // for (i = 0; i < N_THREADS_ORGANIZADORAS; i++) { threadsOrg[i].stop = true; }
    // for (i = 0; i < N_THREADS_EMPRESAS; i++) { threadsEmp[i].stop = true; }

    // 5. Aguardar (join) todas as threads
    pthread_join(threadBuffer.thread, NULL);
    // ... joins para as outras threads

    // 6. Destrói cada mutex por boa prática
    for (int i = 0; i < N_ARQUIVOS; i++) {
        if (pthread_mutex_destroy(&arquivos[i].mutex) != 0) {
            perror("Falha ao destruir mutex");
            exit(EXIT_FAILURE);
        }
    }

    printf("Finalizando.\n");

    return 0;
}
