#include "common.h"

const char *logs[] = { "Interface", "Input", "Operacao", "Localizacao", "Propaganda", "Calculo" };
const char *nomeArquivos[] = { "buffer.log",
                               "Interface.log", "Input.log", "Operacao.log", "Localizacao.log", "Propaganda.log", "Calculo.log",
                               "Omega.log", "KleubsMax.log", "ChirpTome.log"};

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
void moveLog(const char *origem, const char *destino, const char *palavra) {
    char tempName[100];
    strcat(tempName, origem);
    strcat(tempName, "_temp.log");

    FILE *fin = fopen(origem, "r");
    FILE *fout = fopen(destino, "a");
    FILE *ftemp = fopen(tempName, "w");

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
    printf("Thread %d:", t->id_logico);

    for(int i = 0; i < N_ARQUIVOS; i++) {
        printf(" %c:%d", nomeArquivos[i][0], t->acessando[i]);
    }

    printf("\n");
}

// ------
