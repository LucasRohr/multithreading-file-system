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
    const char* tipo_log[N_THREADS_EMPRESAS] = args->log_types;
    int lista_idx_origem[N_THREADS_EMPRESAS] = args->lista_idx_arquivo_origem;
    int idx_destino = args->idx_arquivo_destino;
    
    ArquivoData *arquivoDestino = &arquivos[idx_destino];

    printf("INFO: Thread Organizadora (%s) iniciada.\n", tipo_log);

    while (!myself->stop) {
        // Simula um tempo de "espera" ou "processamento"
        sleep(2);

        // 2. Entra na Região Crítica (tenta travar os mutexes de logs e o da empresa)
        for(int i = 0; i < N_THREADS_EMPRESAS; i++) {
            ArquivoData *arquivoLogs = &arquivos[lista_idx_origem[i]];

            pthread_mutex_lock(&arquivoLogs->mutex);
            myself->acessando[lista_idx_origem[i]] = true;
        }

        pthread_mutex_lock(&arquivoDestino->mutex);

        // 3. Loga o estado atual
        LogThread(myself);

        // 4. Realiza o movimento de logs para cada arquivo de logs que a empresa possui interesse
        for(int i = 0; i < N_THREADS_EMPRESAS; i++) {
            moveLogs(nomeArquivos[lista_idx_origem[i]], nomeArquivos[idx_destino], tipo_log[i]);

            // 5. Sai da Região Crítica
            myself->acessando[lista_idx_origem[i]] = false;
        }

        // 5. Sai da Região Crítica
        myself->acessando[idx_destino] = false;

        // 6. Destravar todos os mutexes
        // Libera os mutexes na ordem inversa em que foram travados pra evitar deadlocks
        pthread_mutex_unlock(&arquivos[idx_destino].mutex);

        for(int i = 0; i < N_THREADS_EMPRESAS; i++) {
            pthread_mutex_unlock(&arquivos[lista_idx_origem[i]].mutex);
        }
    }

    return NULL;
}
