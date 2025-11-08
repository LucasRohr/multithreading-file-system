#include "common.h"

const char *logs[] = { "Interface", "Input", "Operacao", "Localizacao", "Propaganda", "Calculo" };
const char *nomeArquivos[] = { "buffer.log",
                               "Interface.log", "Input.log", "Operacao.log", "Localizacao.log", "Propaganda.log", "Calculo.log",
                               "Omega.log", "KleubsMax.log", "ChirpTome.log"};

const char *threadNames[N_THREADS_TOTAL] = {
    "Produtora",
    "Org_Interface",
    "Org_Operacao",
    "Org_Localizacao",
    "Org_Propaganda",
    "Org_Calculo",
    "Emp_Omega",
    "Emp_KleubsMax",
    "Emp_ChirpTome"
};

// --- Funções auxiliares --

// Função para simular geração de logs com dados randomicos
void geraLogs(const char *name) {
    FILE *f = fopen(name, "a");

    for (int i = 0; i < 5; i++) {
        fprintf(f, "%s\n", logs[rand() % N_TIPO_LOGS]);
    }

    fclose(f);
}

// Copia linhas do arquivo origem para o destino e remove linhas do arquivo de origem
void moveLogs(const char *origem, const char *destino, const char *palavra) {
    char tempName[100];
    strcpy(tempName, origem);
    strcat(tempName, "_temp.log");

    FILE *fin = fopen(origem, "r");
    // Checagem 1
    // Se fin é NULL, o arquivo de origem não existe
    if (fin == NULL) {
        return;
    }

    FILE *fout = fopen(destino, "a");
    // Checagem 2
    if (fout == NULL) {
        perror("moveLogs: Falha ao abrir arquivo de destino");
        fclose(fin); // Fecha o 'fin' que abrimos com sucesso
        return;
    }

    FILE *ftemp = fopen(tempName, "w");
    // Checagem 3
    if (ftemp == NULL) {
        perror("moveLogs: Falha ao abrir arquivo temporário");
        fclose(fin);
        fclose(fout);
        return;
    }

    // Agora é seguro usar fin, fout e ftemp
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

// Função para printar dados de uma thread
void LogThread(ThreadData *t) {
    // Prepara uma string para os nomes dos arquivos
    char arquivos_acessados[256] = {0}; // Buffer para os nomes
    bool primeiro = true;

    // Itera por todos os arquivos
    for (int i = 0; i < N_ARQUIVOS; i++) {
        // Se a thread está acessando, adiciona o nome na string
        if (t->acessando[i]) {
            if (!primeiro) {
                // Adiciona uma vírgula se não for o primeiro arquivo
                strcat(arquivos_acessados, ", ");
            }
            // Adiciona o nome do arquivo
            strcat(arquivos_acessados, nomeArquivos[i]);
            primeiro = false;
        }
    }

    // Imprime o log formatado
    // Se 'primeiro' ainda é true, significa que nenhum arquivo foi acessado
    if (primeiro) {
        printf("LOG: [Thread %s] está ativa (sem travas)\n", threadNames[t->id_logico]);
    } else {
        printf("LOG: [Thread %s] está acessando -> [%s]\n", 
               threadNames[t->id_logico], 
               arquivos_acessados);
    }
}

// ------
