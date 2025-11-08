#include "deadlock.h"

// Tenta encontrar um ciclo de deadlock em um "grafo" de espera de recursos
int encontraCicloDeadlock(ThreadData** all_threads, int* path_ciclo) {
    // Testa iniciar uma "cadeia de espera" a partir de cada thread
    for (int i = 0; i < N_THREADS_TOTAL; i++) {
        int thread_id_inicial = i;
    
        ThreadData* start_thread = all_threads[thread_id_inicial];

        // 1. Se essa thread não está esperando por nada, ela não pode iniciar uma cadeia de deadlock. Vai para a próxima
        if (start_thread->target_arquivo_id == -1) {
            continue;
        }

        // 2. Caso contrário, a thread 'i' está esperando. Segue a cadeia
        // 'caminho_atual' armazena as threads na cadeia de espera
        int caminho_atual[N_THREADS_TOTAL + 1];
        int tamanho_caminho = 0;
        
        int thread_id_atual = thread_id_inicial;

        // 3. Loop para seguir a cadeia de dependências (Enquanto a cadeia não quebrar ou se fechar)
        while (true) {
            // 4. Adiciona a thread atual ao caminho que estamos investigando
            caminho_atual[tamanho_caminho++] = thread_id_atual;

            // 5. Vê qual arquivo essa thread quer
            int arquivo_desejado = all_threads[thread_id_atual]->target_arquivo_id;
            
            // 6. Se ela não quer nada, a cadeia para aqui (não é um ciclo)
            if (arquivo_desejado == -1) {
                break; // Sai do while, vai para a próxima thread i
            }

            // 7. Vê quem está segurando o arquivo desejado
            int thread_id_segurando = arquivos[arquivo_desejado].accessing_thread_id;

            // 8. Se ninguém está segurando, a cadeia para aqui. (Não é um ciclo)
            if (thread_id_segurando == -1) {
                break; // Sai do while
            }

            // 9. Checagem crítica para encontrar um ciclo
            // Verifica se a thread que está segurando o recurso ('thread_id_segurando') já está no caminho atual
            for (int j = 0; j < tamanho_caminho; j++) {
                if (caminho_atual[j] == thread_id_segurando) {
                    // Foi encontrado ciclo
                    // O ciclo começa no índice 'j' do caminho
                    
                    // Ex: caminho_atual = [6, 7, 8] e thread_id_segurando = 6
                    // O ciclo é 6 -> 7 -> 8 -> 6

                    // Copia o ciclo (de [j] até o fim) para o array de saída 'path_ciclo'
                    int k = 0;
                    for (int idx_ciclo = j; idx_ciclo < tamanho_caminho; idx_ciclo++) {
                        path_ciclo[k++] = caminho_atual[idx_ciclo];
                    }

                    path_ciclo[k] = thread_id_segurando; // Fecha o ciclo
                    
                    return k + 1; // Retorna o tamanho do ciclo
                }
            }
            
            // 10. Se não encontrou um ciclo, mas a cadeia ficou maior que número de threads, houve erro e é parado por segurança
            if (tamanho_caminho > N_THREADS_TOTAL) {
                break;
            }

            // 11. Se não é um ciclo, a cadeia continua. A próxima thread a ser checada é a que está segurando o recurso
            thread_id_atual = thread_id_segurando;

        } // fim do while que segue a cadeia
        
    } // fim do for que testa cada thread como início

    // Se o 'for' terminar, quer dizer que testou todas as possibilidades e nenhum ciclo foi encontrado

    return 0;
}

// Imprime o estado do deadlock de forma clara, com threads envolvidas
void printaDadosDeadlock(int* path_ciclo, int path_len, ThreadData** all_threads) {
    printf("Estado de Deadlock (Ciclo encontrado):\n");

    for (int i = 0; i < path_len - 1; i++) {
        int thread_id = path_ciclo[i]; // Pega thread que está aguardando no ciclo
        int target_arquivo_idx = all_threads[thread_id]->target_arquivo_id; // Arquivo que a thread quer
        int proxima_thread_id = path_ciclo[i+1]; // Próxima thread no ciclo
        
        printf("  - [Thread %s] (ID %d) está esperando por [%s]\n",
               threadNames[thread_id], thread_id, nomeArquivos[target_arquivo_idx]);

        printf("      ...que está sendo segurado por [Thread %s] (ID %d)\n",
               threadNames[proxima_thread_id], proxima_thread_id);
    }
}

// Resolve o deadlock quebrando o ciclo das threads escolhendo uma vítima para liberar arquivo
void resolverDeadlock(int* path_ciclo, int path_len, ThreadData** all_threads) {
    // Estratégia: Matar o último thread no ciclo (vítima)
    int vitima_thread_id = path_ciclo[path_len - 2]; // O último antes de fechar o ciclo
    
    printf("Ação de Resolução: [Thread %s] (ID %d) foi escolhida como vítima e será parada.\n",
           threadNames[vitima_thread_id], vitima_thread_id);

    // Seta flag de stop para true.
    // A lógica em lock_arquivo() e na thread garantem que ela pare e libere o arquivo
    all_threads[vitima_thread_id]->stop = true;
}
