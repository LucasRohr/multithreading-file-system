#include "deadlock.h"

// Arquivos globais
ArquivoData arquivos[N_ARQUIVOS];

const char* tipos_organizadoras[] = {
    "Interface", "Operacao", "Localizacao", "Propaganda", "Calculo"
};

// Define os interesses de cada empresa
int fontes_omega[N_THREADS_EMPRESAS] = {IDX_OPERACAO, IDX_PROPAGANDA, IDX_CALCULO};
const char* logs_omega[N_THREADS_EMPRESAS] = {"Operacao", "Propaganda", "Calculo"};

int fontes_kleubsmax[N_THREADS_EMPRESAS] = {IDX_PROPAGANDA, IDX_INTERFACE, IDX_LOCALIZACAO};
const char* logs_kleubsmax[N_THREADS_EMPRESAS] = {"Propaganda", "Interface", "Localizacao"};

// ChirpTome busca 'Input' direto do BUFFER
int fontes_chirptome[N_THREADS_EMPRESAS] = {IDX_CALCULO, IDX_LOCALIZACAO, IDX_BUFFER};
const char* logs_chirptome[N_THREADS_EMPRESAS] = {"Calculo", "Localizacao", "Input"};

int idx_destino_organizadoras[] = {
    IDX_INTERFACE, IDX_OPERACAO, IDX_LOCALIZACAO, IDX_PROPAGANDA, IDX_CALCULO
};

// Para tratamento de deadlocks:

// 1) Detecta o deadlock (a thread main checa a cada 5 segundos )
// 2) Imprime o estado (quais threads e arquivos estão envolvidos )
// 3) Resolve o deadlock

// Solução:

// 1) Trocar todo pthread_mutex_lock por um loop com pthread_mutex_trylock para retornar se não houver lock
// 2) Detectar quais thread estão esperando por arquivos na main
// 3) Resolver o ciclo na main escolhendo uma thread vítima para se parada e dar lugar à outra thread no aguardo

int main() {
    srand(time(NULL));
    printf("Iniciando...\n");

    // 1. Inicializa os arquivos e mutexes
    for (int i = 0; i < N_ARQUIVOS; i++) {
        arquivos[i].id = i;
        arquivos[i].accessing_thread_id = -1; // Nenhuma thread querendo acessar o arquivo ainda

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
    ThreadEmpresaArgs argsEmpresa[N_THREADS_EMPRESAS];

    ThreadData* all_threads[N_THREADS_TOTAL]; // Lista de threads para deadlock

    // Inicializar e criar a Thread do Buffer
    threadBuffer.id_logico = 0;
    threadBuffer.stop = false;
    threadBuffer.target_arquivo_id = -1; // Thread ainda não quer acessar nenhum arquivo

    for (int j = 0; j < N_ARQUIVOS; j++) {
        threadBuffer.acessando[j] = false;
    }

    all_threads[0] = &threadBuffer; // Primeira thread é o buffer

    pthread_create(&threadBuffer.thread, NULL, ThreadBuffer, &threadBuffer);

    printf("Thread do Buffer criada\n");

    // Inicializar e criar as 5 Threads Organizadoras (tipos de dados com logs)
    printf("Criando threads organizadoras...\n");
    for (int i = 0; i < N_THREADS_ORGANIZADORAS; i++) {
        threadsOrg[i].id_logico = i + 1;
        threadsOrg[i].stop = false;
        threadsOrg[i].target_arquivo_id = -1; // Thread ainda não quer acessar nenhum arquivo

        all_threads[i + 1] = &threadsOrg[i]; // Atribui thread na lista

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
    printf("Criando threads das empresas...\n");

    // Empresa 0: Ômega
    threadsEmp[0].id_logico = 6; // IDs 6, 7, 8
    threadsEmp[0].stop = false;
    threadsEmp[0].target_arquivo_id = -1; // Thread ainda não quer acessar nenhum arquivo

    for (int j = 0; j < N_ARQUIVOS; j++) {
        threadsEmp[0].acessando[j] = false;
    }

    all_threads[6] = &threadsEmp[0]; // Atribui thread na lista

    argsEmpresa[0].data = &threadsEmp[0];
    argsEmpresa[0].idx_arquivo_destino = IDX_OMEGA;
    argsEmpresa[0].lista_idx_arquivo_origem = fontes_omega;
    argsEmpresa[0].log_types = logs_omega;
    argsEmpresa[0].num_fontes = 3;
    pthread_create(&threadsEmp[0].thread, NULL, ThreadEmpresa, &argsEmpresa[0]);

    // Empresa 1: KleubsMax
    threadsEmp[1].id_logico = 7;
    threadsEmp[1].stop = false;
    threadsEmp[1].target_arquivo_id = -1; // Thread ainda não quer acessar nenhum arquivo

    for (int j = 0; j < N_ARQUIVOS; j++) {
        threadsEmp[1].acessando[j] = false;
    }

    all_threads[7] = &threadsEmp[1]; // Atribui thread na lista

    argsEmpresa[1].data = &threadsEmp[1];
    argsEmpresa[1].idx_arquivo_destino = IDX_KLEUBSMAX;
    argsEmpresa[1].lista_idx_arquivo_origem = fontes_kleubsmax;
    argsEmpresa[1].log_types = logs_kleubsmax;
    argsEmpresa[1].num_fontes = 3;
    pthread_create(&threadsEmp[1].thread, NULL, ThreadEmpresa, &argsEmpresa[1]);

    // Empresa 2: ChirpTome
    threadsEmp[2].id_logico = 8;
    threadsEmp[2].stop = false;
    threadsEmp[2].target_arquivo_id = -1; // Thread ainda não quer acessar nenhum arquivo

    for (int j = 0; j < N_ARQUIVOS; j++) {
        threadsEmp[2].acessando[j] = false;
    }

    all_threads[8] = &threadsEmp[2]; // Atribui thread na lista

    argsEmpresa[2].data = &threadsEmp[2];
    argsEmpresa[2].idx_arquivo_destino = IDX_CHIRPTOME;
    argsEmpresa[2].lista_idx_arquivo_origem = fontes_chirptome;
    argsEmpresa[2].log_types = logs_chirptome;
    argsEmpresa[2].num_fontes = 3;
    pthread_create(&threadsEmp[2].thread, NULL, ThreadEmpresa, &argsEmpresa[2]);

    // 3. Loop principal (Parte 2: Detecção de Deadlock)

    printf("\n*** INICIANDO DETECTOR DE DEADLOCK (Parte 2) ***\n");
    int path_ciclo[N_THREADS_TOTAL + 1]; // Armazena o ciclo ex: T1 -> T2 -> T1
    int path_len = 0;

    for (int i = 0; i < 12; i++) { // Roda por 60 segundos (12 * 5s)
        sleep(5);

        path_len = encontraCicloDeadlock(all_threads, path_ciclo);

        if (path_len > 0) {
            printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            printf(">>> DEADLOCK DETECTADO <<<\n");

            // Imprime o estado 
            printaDadosDeadlock(path_ciclo, path_len, all_threads);

            // Resolve o deadlock 
            resolverDeadlock(path_ciclo, path_len, all_threads);

            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

            // Dá um tempo para a vítima se encerrar
            sleep(2); 
        } else {
            printf("[Detector] Ciclo %d/12: Nenhum deadlock encontrado.\n", i + 1);
        }
    }

    // 4. Sinalizar parada para todas as threads
    printf("Parando todas as threads restantes...\n");

    for (int i = 0; i < N_THREADS_TOTAL; i++) {
        all_threads[i]->stop = true;
    }

    // 5. Aguardar (join) todas as threads
    for (int i = 0; i < N_THREADS_TOTAL; i++) {
        pthread_join(all_threads[i]->thread, NULL);
    }

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
