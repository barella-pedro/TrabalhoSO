#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include "aviao.h"
//LEMBRAR COLOCAR CADA FUNÇÃO UTILIZADA POR CADA INCLUDE - PODE AJUDAR A ESCREVER O RELATÓRIO E REMOVER INCLUDES DESNECESSÁRIOS
//TIRAR DPS
#define N 1

void testa_mem(int segmento);
void teste_attach(Aviao* frota);

int main(){
    int pid_f, i;
    int segmento;
    /*INICIALIZAÇÃO DA MEMÓRIA COMPARTILHADA */
    Aviao* frota;
    segmento = shmget(IPC_PRIVATE, N * sizeof(Aviao), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    testa_mem(segmento);
    frota = (Aviao*)shmat(segmento, NULL, 0);
    teste_attach(frota);
    /*****************************************/
    
    for (i = 0; i < N; i++){
        if ((pid_f = fork()) < 0 ){
            perror("Erro ao criar processo filho\n");
            exit(1);
        }
        else if (pid_f == 0){
            char indice[10], id_shm[10];
            sprintf(indice, "%d", i);//transformando i em string para entrar no argv de voar
            sprintf(id_shm, "%d", segmento);//transformando o i
            printf("%s\n",indice);printf("%s\n",id_shm);
            sleep(2);
            execl("./voar", "voar", indice, id_shm, NULL);
            exit(1);
        }
    }
    printf("ID da Frota (shm) = %d\n\n", segmento);
    

    int voo_atual = 0;
    int voos_ativos = N;
    sleep(3);
    while (voos_ativos > 0) {
        if (frota[voo_atual].status == STATUS_VOANDO) {
            printf("While 1 do pai\n");
            // Acorda
            kill(frota[voo_atual].pid, SIGCONT);
            sleep(5); // tempo de "voo"
            kill(frota[voo_atual].pid, SIGSTOP);
            
            printf("Local do aviao %d: %f %f\n",voo_atual,frota[voo_atual].x, frota[voo_atual].y);
            // Verifica se pousou
            if (frota[voo_atual].x == 0.5f && frota[voo_atual].y == 0.5f) {
                printf("Controlador: avião PID %d pousou.\n", frota[voo_atual].pid);
                frota[voo_atual].status = STATUS_ATERRISSADO;
                voos_ativos--;
            }
        } else if (frota[voo_atual].status == STATUS_ATERRISSADO || frota[voo_atual].status == STATUS_REMOVIDO) {
            // Já não está voando, ignora
            // printf("Avião %d já finalizado\n", atual);
        }

        voo_atual = (voo_atual + 1) % N;
    }


    //Desanexando e limpando memoria compartilhada
    for (i = 0; i < N; i++){
        wait(NULL);
        
    }
    shmdt(frota);
    shmctl(segmento, IPC_RMID, 0);
    return 0;
}

void testa_mem(int segmento){
    if (segmento == -1){
        perror("Erro ao criar a memoria compartilhada!\n");
        exit(-1);
    }
    return;
}