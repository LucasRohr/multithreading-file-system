#include "common.h"
#include <errno.h> // 'EBUSY' do trylock

// --- Funções auxiliares de Lock/Unlock de mutexes ---

// Tenta travar um mutex. Retorna true no sucesso, false se for vítima
static bool lock_arquivo(ThreadData* myself, int idx_arquivo) {
    myself->target_arquivo_id = idx_arquivo;

    while (true) {
        // Tenta pegar o lock
        int return_lock = pthread_mutex_trylock(&arquivos[idx_arquivo].mutex);

        if (return_lock == 0) {
            // Sucesso no lock
            myself->target_arquivo_id = -1; // Reseta arquivo alvo
            myself->acessando[idx_arquivo] = true; // Está acessando
            arquivos[idx_arquivo].accessing_thread_id = myself->id_logico; // Atribui thread que está acessando arquivo na sua estrutura
            return true;
        
        } else if (return_lock == EBUSY) {
            // O lock está ocupado, continua esperando
            
            // Mas checam se foi escolhido como vítima
            if (myself->stop) {
                printf("WARNING: [Thread %s] detectou 'stop' (vítima) enquanto esperava por %s\n",
                       threadNames[myself->id_logico], nomeArquivos[idx_arquivo]);
                myself->target_arquivo_id = -1;
                return false; // Falha (vítima)
            }
            
            // Espera um pouco antes de tentar o lock de novo
            usleep(100 * 1000);
        } else {
            // Erro default
            perror("pthread_mutex_trylock");
            myself->stop = true; // Força a parada caso tenha erro
            return false;
        }
    }
}

// Libera um mutex
static void unlock_arquivo(ThreadData* myself, int idx_arquivo) {
    if (myself->acessando[idx_arquivo]) {
        myself->acessando[idx_arquivo] = false; // Libera flag de acesso
        arquivos[idx_arquivo].accessing_thread_id = -1; // Reseta arquivo alvo
        pthread_mutex_unlock(&arquivos[idx_arquivo].mutex);
    }
}

// --- Fim das auxiliares ---

// Abaixo, as threads agora usam as funções aux pro tratamento de deadlocks

// A thread do buffer de dados de input
void *ThreadBuffer(void *arg) {
    ThreadData* myself = (ThreadData*)arg;
    int idx_buffer = IDX_BUFFER;

    while(!myself->stop) {
        // Tenta travar o buffer, se não conseguir, foi escolhido como vítima
        if (!lock_arquivo(myself, idx_buffer)) {
            break;
        }

        LogThread(myself); // Printa dados
        geraLogs(nomeArquivos[idx_buffer]); // Gera logs no buffer

        // Libera o buffer
        unlock_arquivo(myself, idx_buffer);

        sleep(1);
    }

    liberarTodosLocks(myself); // Limpeza final
    printf("INFO: Thread Buffer finalizando.\n");

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

    printf("INFO: Thread Organizadora (%s) iniciada.\n", tipo_log);

    while(!myself->stop) {
        // Simula um tempo de "espera" ou "processamento"
        sleep(2);

        // Trava o buffer, se não conseguir, foi escolhido como vítima
        if (!lock_arquivo(myself, idx_origem)) {
            break; // Vítima
        }
        
        // Trava o arquivo de destino, se não conseguir, foi escolhido como vítima
        if (!lock_arquivo(myself, idx_destino)) {
            break; // Vítima
        }

        LogThread(myself); // Printa dados
        moveLogs(nomeArquivos[idx_origem], nomeArquivos[idx_destino], tipo_log); // Moves logs para arquivo destino


        // Libera os mutexes na ordem inversa em que foram travados pra evitar deadlocks
        unlock_arquivo(myself, idx_destino);
        unlock_arquivo(myself, idx_origem);
    }

    liberarTodosLocks(myself); // Limpeza final
    printf("INFO: Thread Organizadora (%s) finalizando.\n", tipo_log);

    return NULL;
}

void *ThreadEmpresa(void *arg) {
    ThreadEmpresaArgs* args = (ThreadEmpresaArgs*)arg;
    ThreadData* myself = args->data;
    int idx_destino = args->idx_arquivo_destino;
    int num_fontes = args->num_fontes;

    printf("INFO: Thread Empresa (ID %d) iniciada. Destino: %s\n", myself->id_logico, nomeArquivos[idx_destino]);

    while (!myself->stop) {
        // Simula um tempo de espera/processamento randomico
        sleep(rand() % 5 + 1);

        // Tenta travar todos os mutexes necessários, um por um usando lock_arquivo

        bool travou_todos_arquivos = true;

        // Tenta travar os mutexes dos arquivos de origem, se não conseguir, é vítima
        for (int i = 0; i < args->num_fontes; i++) {
            if (!lock_arquivo(myself, args->lista_idx_arquivo_origem[i])) {
                travou_todos_arquivos = false;
                break;
            }
        }

        if (!travou_todos_arquivos) break;

        // Tenta travar o arquivo de destino, se não conseguir, é vítima
        if (!lock_arquivo(myself, args->idx_arquivo_destino)) {
            break;
        }

        LogThread(myself); // Printa dados

        // Realiza o movimento de logs para cada arquivo que a empresa possui interesse, para o arquivo da empresa
        for (int i = 0; i < num_fontes; i++) {
            int idx_fonte = args->lista_idx_arquivo_origem[i];
            const char* log_tipo = args->log_types[i];

            moveLogs(nomeArquivos[idx_fonte], nomeArquivos[idx_destino], log_tipo); // Move da fonte para o arquivo da empresa
        }

        // Libera todos os mutexes (em ordem inversa)
        unlock_arquivo(myself, idx_destino); // Libera do arquivo destino

        // Libera de cada arquivo de interesse da empresa
        for (int i = args->num_fontes - 1; i >= 0; i--) {
            unlock_arquivo(myself, args->lista_idx_arquivo_origem[i]);
        }
    }

    liberarTodosLocks(myself); // Limpeza final
    printf("INFO: Thread Empresa (ID %d) finalizando.\n", myself->id_logico);

    return NULL;
}
