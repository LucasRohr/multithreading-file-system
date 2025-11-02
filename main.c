#include "common.h"

// Arquivos globais
ArquivoData arquivos[N_ARQUIVOS];

const char* tipos_organizadoras[] = {
    "Interface", "Operacao", "Localizacao", "Propaganda", "Calculo"
};

int idx_destino_organizadoras[] = {
    IDX_INTERFACE, IDX_OPERACAO, IDX_LOCALIZACAO, IDX_PROPAGANDA, IDX_CALCULO
};

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

    ThreadOrganizadoraArgs argsOrganizadora[N_THREADS_ORGANIZADORAS];

    // Inicializar e criar a Thread do Buffer
    threadBuffer.id_logico = 0;
    threadBuffer.stop = false;
    pthread_create(&threadBuffer.thread, NULL, ThreadBuffer, &threadBuffer);

    printf("Thread do Buffer criada\n");

    // Inicializar e criar as 5 Threads Organizadoras (tipos de dados com logs)
    printf("Criando threads organizadoras...\n");
    for (int i = 0; i < N_THREADS_ORGANIZADORAS; i++) {
        threadsOrg[i].id_logico = i + 1;
        threadsOrg[i].stop = false;

        for (int j = 0; j < N_ARQUIVOS; j++) {
            threadsOrg[i].acessando[j] = false;
        }

        // Argumentos da thread organizadora
        argsOrganizadora[i].data = &threadsOrg[i];
        argsOrganizadora[i].log_type = tipos_organizadoras[i];
        argsOrganizadora[i].idx_arquivo_origem = IDX_BUFFER; // Sempre o ID do buffer
        argsOrganizadora[i].idx_arquivo_destino = idx_destino_organizadoras[i];

        // Cria a thread
        if (pthread_create(&threadsOrg[i].thread, NULL, ThreadOrganizadora, &argsOrganizadora[i]) != 0) {
            perror("Falha ao criar thread organizadora");
            exit(EXIT_FAILURE);
        }

        printf("Thread organizadora de logs [%s] criada\n", argsOrganizadora[i].log_type);
    }

    // Inicializar e criar as 3 Threads das Empresas
    // (Também precisarão de argumentos)
    // for (i = 0; i < N_THREADS_EMPRESAS; i++) { ... }

    // 3. Loop principal
    // Por enquanto, apenas um sleep
    sleep(10);

    // 4. Sinalizar parada para todas as threads
    printf("Parando threads...\n");

    threadBuffer.stop = true;

    for (int i = 0; i < N_THREADS_ORGANIZADORAS; i++) {
        threadsOrg[i].stop = true;
    }

    // for (i = 0; i < N_THREADS_EMPRESAS; i++) { threadsEmp[i].stop = true; }

    // 5. Aguardar (join) todas as threads
    pthread_join(threadBuffer.thread, NULL);

    for (int i = 0; i < N_THREADS_ORGANIZADORAS; i++) {
        pthread_join(threadsOrg[i].thread, NULL);
    }

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
