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

// --- Funções para a Parte 3 ---

// Função para destravar uma lista de arquivos
static void unlock_multiplos_arquivos(ThreadData* myself, int* indices_arquivos, int num_arquivos) {
    for (int i = 0; i < num_arquivos; i++) {
        int idx = indices_arquivos[i];

        if (myself->acessando[idx]) { // Se estiver acessando arquivo
            myself->acessando[idx] = false; // Reseta flag
            arquivos[idx].accessing_thread_id = -1; // Reseta thread que acessa arquivo
            pthread_mutex_unlock(&arquivos[idx].mutex); // Destrava mutex
        }
    }
}

// Função que tenta travar todos os arquivos necessários para uma thread que quer ter acesso
// Caso não conseguir travar algum, libera todos os travados anteriormente
// Retorna true se travou todos e false caso contrário
static bool lock_multiplos_arquivos_prevencao(ThreadData* myself, int* indices_arquivos, int num_arquivos) {
    // Tenta pegar todos os locks
    for (int i = 0; i < num_arquivos; i++) {
        int idx = indices_arquivos[i];
        myself->target_arquivo_id = idx; // Seta arquivo alvo da thread

        int return_lock = pthread_mutex_trylock(&arquivos[idx].mutex); // Tenta travar arquivo

        if (return_lock == EBUSY) {
            // Falhou pois o arquivo está ocupado
            // Então destrava todos travados até aqui
            myself->target_arquivo_id = -1;
            
            for (int j = 0; j < i; j++) {
                pthread_mutex_unlock(&arquivos[indices_arquivos[j]].mutex);
            }

            return false; // Falhou travar todos
        } else if (return_lock != 0) {
            // Outro erro, falha também
            perror("lock_multiplos_prevencao: trylock");

            return false;
        }
    }

    // Se chegou aqui, conseguiu fazer lock em todos os arquivos e atualiza a lista de acessos
    myself->target_arquivo_id = -1;

    for (int i = 0; i < num_arquivos; i++) {
        int idx = indices_arquivos[i];
    
        myself->acessando[idx] = true; // Seta flag de acesso
        arquivos[idx].accessing_thread_id = myself->id_logico; // Seta thread que está acessando o arquivo
    }

    return true; // Sucesso ao travar todos
}

// Função para a thread das empresas focando na prevenção de deadlocks para a Parte 3
void *ThreadEmpresa_Prevencao(void *arg) {
    ThreadEmpresaArgs* args = (ThreadEmpresaArgs*)arg;
    ThreadData* myself = args->data;
    int idx_destino = args->idx_arquivo_destino;
    int num_fontes = args->num_fontes;

    // Monta array único com todos os arquivos que precisam ser travados (fontes + destino)
    int num_total_arquivos = num_fontes + 1;
    int* todos_indices = malloc(sizeof(int) * num_total_arquivos);

    if (todos_indices == NULL) {
        perror("ThreadEmpresa_Prevencao: Falha ao alocar memória");
        return NULL;
    }

    // Adiciona arquivos de fonte no array
    for (int i = 0; i < num_fontes; i++) {
        todos_indices[i] = args->lista_idx_arquivo_origem[i];
    }

    todos_indices[num_fontes] = idx_destino; // Adiciona o arquivo de destino ao array

    printf("INFO: Thread Empresa [PREVENÇÃO] (ID %d) iniciada.\n", myself->id_logico);

    while (!myself->stop) {
        sleep(rand() % 5 + 1);

        // Tenta dar lock em todos os arquivos (ou nenhum, se falhar em algum) em um loop
        while (!lock_multiplos_arquivos_prevencao(myself, todos_indices, num_total_arquivos)) {
            // Se falhou, checa se deve parar
            if (myself->stop) {
                free(todos_indices);
                return NULL; 
            }

            // Se chegou aqui, não conseguiu todos os locks, espera e tenta de novo
            usleep(100 * 1000); // 100ms
        }

        // Região crítica
        // Se chegou aqui, tem todos os locks necessários, então não haver ter deadlock
        
        LogThread(myself); // Printa dados

        // Realiza o movimento de logs para cada arquivo que a empresa possui interesse, para o arquivo da empresa
        for (int i = 0; i < num_fontes; i++) {
            int idx_fonte = args->lista_idx_arquivo_origem[i];
            const char* log_tipo = args->log_types[i];

            moveLogs(nomeArquivos[idx_fonte], nomeArquivos[idx_destino], log_tipo);
        }

        // Libera todos os arquivos
        unlock_multiplos_arquivos(myself, todos_indices, num_total_arquivos);
    }

    free(todos_indices);
    printf("INFO: Thread Empresa [PREVENÇÃO] (ID %d) finalizando.\n", myself->id_logico);

    return NULL;
}
