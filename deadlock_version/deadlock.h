#include "common.h"

int encontraCicloDeadlock(ThreadData** all_threads, int* path_ciclo);
void printaDadosDeadlock(int* path_ciclo, int path_len, ThreadData** all_threads);
void resolverDeadlock(int* path_ciclo, int path_len, ThreadData** all_threads);
