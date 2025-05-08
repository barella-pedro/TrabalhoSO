#ifndef AVIAO_H
#define AVIAO_H

#include <unistd.h> // Para pid_t

#define STATUS_VOANDO 1
#define STATUS_PAUSADO  2
#define STATUS_ATERRISSADO  3
#define STATUS_REMOVIDO 4

/*


1110 1111

0000 0000
0001 0000

( ((i & 0xffffffef)) == 0 && (i != 0x00) && (i != 0x10) )


*/

typedef struct aviao {
    pid_t pid;
    int pista_destino;
    int status; 
    float atraso;
    float x;
    float y;
    float velocidade; // Velocidade em Y. A velocidade em X é sempre 0.05;
    char direcao; // Deve ser 'W' ou 'E'
} Aviao;

void teste_attach(Aviao* frota);
#endif