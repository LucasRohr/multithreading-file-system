#include "common.h"

void *ThreadBuffer(void *arg) {
    ThreadData* myself = (ThreadData*)arg;
    ArquivoData *d = &arquivos[IDX_BUFFER];

    while(!myself->stop) {
        pthread_mutex_lock(&d->mutex);
        myself->acessando[IDX_BUFFER] = true;
        LogThread(myself);
        
        geraLogs(nomeArquivos[d->id]);

        myself->acessando[IDX_BUFFER] = false;
        pthread_mutex_unlock(&d->mutex);
        sleep(1);
    }

    return NULL;
}

void *ThreadOrganizadora(void *arg) {
    // Esta thread precisará saber qual log ela procura.
    // Você pode passar um struct mais complexo como 'arg'.
    // Exemplo: { ThreadData* data, const char* tipo_log, int idx_arquivo_log }
    
    // Loop:
    // 1. Travar mutex do buffer.log
    // 2. Travar mutex do seu arquivo de destino (ex: Interface.log)
    // 3. Chamar moveLog(...)
    // 4. Destravar ambos os mutexes
    // 5. sleep(...)
    return NULL;
}

void *ThreadEmpresa(void *arg) {
    // Lógica mais complexa.
    // Ex: Thread Ômega [cite: 137]
    // Loop:
    // 1. Travar mutex do Operacao.log
    // 2. Travar mutex do Propaganda.log
    // 3. Travar mutex do Calculo.log
    // 4. Travar mutex do Omega.log
    // 5. Chamar moveLog 3x
    // 6. Destravar os 4 mutexes
    // 7. sleep(...)
    return NULL;
}
