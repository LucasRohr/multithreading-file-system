#include "common.h"

// A thread do buffer de dados de input
void *ThreadBuffer(void *arg) {
    ThreadData* myself = (ThreadData*)arg;
    ArquivoData *d = &arquivos[IDX_BUFFER];

    while(!myself->stop) {
        pthread_mutex_lock(&d->mutex); // Trava thread
        myself->acessando[IDX_BUFFER] = true;
        LogThread(myself); // Printa dados
        
        geraLogs(nomeArquivos[d->id]); // Gera logs no buffer

        myself->acessando[IDX_BUFFER] = false;
        pthread_mutex_unlock(&d->mutex); // Destrava thread
        sleep(1);
    }

    return NULL;
}

// Função para gerenciamento do acesso a um arquivo de logs via thread dedicada e thread do buffer
void *ThreadOrganizadora(void *arg) {
    // 1. Recupera os argumentos
    ThreadOrganizadoraArgs* args = (ThreadOrganizadoraArgs*)arg;
    ThreadData* myself = args->data;
    const char* tipo_log = args->log_type;
    int idx_origem = args->idx_arquivo_origem;
    int idx_destino = args->idx_arquivo_destino;
    
    ArquivoData *arquivoBuffer = &arquivos[idx_origem];
    ArquivoData *arquivoLogs = &arquivos[idx_destino];

    printf("INFO: Thread Organizadora (%s) iniciada.\n", tipo_log);

    while(!myself->stop) {
        // Simula um tempo de "espera" ou "processamento"
        sleep(2);

        // 2. Entra na Região Crítica (tenta travar os dois mutexes)

        // Trava mutex do buffer
        pthread_mutex_lock(&arquivoBuffer->mutex);
        myself->acessando[idx_origem] = true;

        // Trava mutex do seu arquivo de destino (ex: Interface.log)
        pthread_mutex_lock(&arquivoLogs->mutex);
        myself->acessando[idx_destino] = true;

        // 3. Loga o estado atual
        LogThread(myself);

        // 4. Realiza o movimento de logs
        moveLogs(nomeArquivos[idx_origem], nomeArquivos[idx_destino], tipo_log);

        // 5. Sai da Região Crítica
        myself->acessando[idx_origem] = false;
        myself->acessando[idx_destino] = false;

        // 6. Destravar ambos os mutexes
        // Libera os mutexes na ordem inversa em que foram travados pra evitar deadlocks
        pthread_mutex_unlock(&arquivos[idx_destino].mutex);
        pthread_mutex_unlock(&arquivos[idx_origem].mutex);
    }

    printf("INFO: Thread Organizadora (%s) finalizando.\n", tipo_log);

    return NULL;
}

void *ThreadEmpresa(void *arg) {
     // 1. Recupera os argumentos
    ThreadEmpresaArgs* args = (ThreadEmpresaArgs*)arg;
    ThreadData* myself = args->data;
    int idx_destino = args->idx_arquivo_destino;
    int num_fontes = args->num_fontes;
    
    ArquivoData *arquivoDestino = &arquivos[idx_destino];

    printf("INFO: Thread Empresa (ID %d) iniciada. Destino: %s\n", myself->id_logico, nomeArquivos[idx_destino]);

    while (!myself->stop) {
        // Simula um tempo de espera/processamento randomico
        sleep(rand() % 5 + 1);

        // 2. Tenta travar todos os mutexes necessários, um por um (parte vulnerável a deadlock)

        // Trava os mutexes dos arquivos de origem
        for (int i = 0; i < num_fontes; i++) {
            int idx_fonte = args->lista_idx_arquivo_origem[i];

            pthread_mutex_lock(&arquivos[idx_fonte].mutex);
            myself->acessando[idx_fonte] = true;
        }

        // Trava o arquivo de destino
        pthread_mutex_lock(&arquivoDestino->mutex);
        myself->acessando[idx_destino] = true;

        // 3. Loga o estado atual (agora com todos os arquivos travados)
        LogThread(myself);

        // 4. Realiza o movimento de logs para cada arquivo que a empresa possui interesse, para o arquivo da empresa
        for (int i = 0; i < num_fontes; i++) {
            int idx_fonte = args->lista_idx_arquivo_origem[i];
            const char* log_tipo = args->log_types[i];

            moveLogs(nomeArquivos[idx_fonte], nomeArquivos[idx_destino], log_tipo); // Move da fonte para o arquivo da empresa
        }

        // 5. Libera todos os mutexes (em ordem inversa)
        myself->acessando[idx_destino] = false;
        pthread_mutex_unlock(&arquivos[idx_destino].mutex); // Libera do destino

        // Libera de cada arquivo fonte travado anteriormente
        for (int i = num_fontes - 1; i >= 0; i--) {
            int idx_fonte = args->lista_idx_arquivo_origem[i];
            myself->acessando[idx_fonte] = false;

            pthread_mutex_unlock(&arquivos[idx_fonte].mutex);
        }
    }

    printf("INFO: Thread Empresa (ID %d) finalizando.\n", myself->id_logico);

    return NULL;
}
