#include "common.h"

const char *logs[] = { "Interface", "Input", "Operacao", "Localizacao", "Propaganda", "Calculo" };
const char *nomeArquivos[] = { "buffer.log",
                               "Interface.log", "Input.log", "Operacao.log", "Localizacao.log", "Propaganda.log", "Calculo.log",
                               "Omega.log", "KleubsMax.log", "ChirpTome.log"};

// --- Funções auxiliares --
void geraLogs(const char *name) {
    FILE *f = fopen(name, "a");

    for (int i = 0; i < 5; i++) {
        fprintf(f, "%s\n", logs[rand() % N_TIPO_LOGS]);
    }

    fclose(f);
}

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

void LogThread(ThreadData *t) {
    printf("Thread %d:");

    for(int i = 0; i < N_ARQUIVOS; i++) {
        printf(" %c:%d", nomeArquivos[i][0], t->acessando[i]);
    }

    printf("\n");
}

// ------
