#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <signal.h>
#include <math.h>
#include "aviao.h"
void trata_sigcont(int sinal){
    printf("Torre liberou! Continuando o Voo..\n");

    return ;
}
int main(int argc, char* argv[]){
    if (argc != 3) {
        fprintf(stderr, "Uso correto: voar <indice> <shm_id>\n");
        exit(1);
    }
    int indice = atoi(argv[1]);
    int segmento = atoi(argv[2]);
    Aviao* frota = (Aviao*)shmat(segmento, NULL, 0);
    teste_attach(frota);
    Aviao* mem_aviao = &frota[indice];
    
    
    signal(SIGCONT, trata_sigcont);
    srand((getpid() + indice)); //MUDAR DEPOIS!!!
    float atraso, x, y;
    char direcao;
    int pid_aviao;
    int pista_destino;
    pid_aviao = getpid(); //armazena o pid do voo;
    atraso = ((float)rand() / RAND_MAX) * 2.0f; //numero aleatorio de 0 a 2
    y = ((float)rand() / RAND_MAX);
    direcao = (rand() % 2 == 0) ? 'W' : 'E'; // Escolher aleatoriamente entre W e E, utilizando resto de 2.
    printf("%c\n\n",direcao);
    if (direcao == 'E') {
        x = 1.0f;
        pista_destino = rand() % 2;//Pistas 0 e 1 para W

    }
    else if (direcao == 'W') {
        x = 0.0f;
        pista_destino = 2 + (rand() % 2);// Pistas 2 e 3 para E

        
    }
    printf("Indice no voar: %d\n",indice);
    printf("Segmento no voar: %d\n",segmento);

    frota[indice].pid = pid_aviao;
    frota[indice].x = x;
    frota[indice].y = y;
    frota[indice].pista_destino = pista_destino;
    frota[indice].atraso = atraso;
    frota[indice].direcao = direcao;
    frota[indice].status = STATUS_VOANDO;
    frota[indice].velocidade = 0.0f; // inicialmente o aviao está parado em y
    printf("Aviao criado: Indice %d - atraso: %.2f,x = %.2f, y = %.2f, direcao = %c, pista = %d\n", 
    indice, frota[indice].atraso,frota[indice].x, frota[indice].y, frota[indice].direcao, frota[indice].pista_destino);
    pause();//Aviao criado, agora esperamos o Processo Pai "continuar" o filho por Round Robin   

    
    while(1){
        sleep(1);
        printf("while 1 do filho!\n");
        printf("Voando!\n");
        //movimento horizontal(em x)
        if ((mem_aviao->direcao == 'W') && ((mem_aviao->x) < 0.5)) {
            mem_aviao->x += 0.05f; // vem da esquerda para o centro
        } 
        else if ((mem_aviao->direcao == 'E') && ((mem_aviao->x) > 0.5)) {
            
            mem_aviao->x -= 0.05f; // vem da direita para o centro
        }
        //movimento vertical(em y)
        if (mem_aviao->y < 0.5f){
            puts("Y abaixo do aviao!\n");
            mem_aviao->y += 0.01f; // se o aviao iniciar abaixo do aeroporto  
            printf("Pos Y:%f\n",mem_aviao->y);
        }  
        else if (mem_aviao->y > 0.5f){
            puts("Y abaixo do aviao!\n");
            mem_aviao->y -= 0.01f; //se iniciar acima do aeroporto  
            printf("Pos Y:%f\n",mem_aviao->y);
        } 

        //pousando
        if (fabs(mem_aviao->x - 0.5f) < 0.01) mem_aviao->x = 0.5f; //Definindo um valor minimo de proximidade
        if (fabs(mem_aviao->y - 0.5f) < 0.01) mem_aviao->y = 0.5f; //Fazendo-se o mesmo para y
        if (mem_aviao->x == 0.5f && mem_aviao->y == 0.5f){
            mem_aviao->status = STATUS_ATERRISSADO;
            printf("Avião PID %d pousou com sucesso!\n", mem_aviao->pid);
            shmdt(frota);
            exit(0); 
        }
    // pousou!
    }
    return 0;
}
    
    