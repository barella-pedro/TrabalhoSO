#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <math.h>
#include "aviao.h"
//LEMBRAR COLOCAR CADA FUNÇÃO UTILIZADA POR CADA INCLUDE - PODE AJUDAR A ESCREVER O RELATÓRIO E REMOVER INCLUDES DESNECESSÁRIOS
//TIRAR DPS
#define N 5

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

    int pids[N];       //Vetor para armazenar os pids    
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
            execl("./voar2", "voar2", indice, id_shm, NULL);
            exit(1);
        }
            else {
            pids[i] = pid_f;
        }
    }

        
    printf("ID da Frota (shm) = %d\n\n", segmento);
    

    int voo_atual = 0;
    int voos_ativos = N;
    sleep(3);

    while (voos_ativos > 0) { //Round Robin
        
        if (frota[voo_atual].status == STATUS_VOANDO) 
    {

                                 
        for (int j = 0; j < N; j++) // Verificando colisão crítica perto do aeroporto.
{
            if (j == voo_atual) continue;
            if (frota[j].status != STATUS_VOANDO) continue;

            if (frota[j].direcao != frota[voo_atual].direcao) continue;

            float dx = frota[voo_atual].x - frota[j].x;
            float dy = frota[voo_atual].y - frota[j].y;
            float dist2 = dx * dx + dy * dy;

            // Se ambos estão perto do centro do aeroporto
            float dx_pista = frota[voo_atual].x - 0.5f;
            float dy_pista = frota[voo_atual].y - 0.5f;
            float dist_pista2 = dx_pista * dx_pista + dy_pista * dy_pista;

            if (dist2 < 0.01f && dist_pista2 < 0.01f) // < 0.1^2 de outro avião e da pista
            { 
                if (frota[voo_atual].pista_destino == frota[j].pista_destino) 
                {
                    printf("Conflito crítico na mesma pista (%d) entre %d e %d. Matando %d.\n",
                    frota[j].pista_destino, j, voo_atual, voo_atual);
                    kill(frota[voo_atual].pid, SIGKILL);
                    frota[voo_atual].status = STATUS_REMOVIDO;
                    voos_ativos--;
                    goto proximo_voo;
                }       
                else 
                {
                        printf("✅ Aviões %d e %d próximos, mas em pistas diferentes. Sem conflito.\n", voo_atual, j);
                        goto proximo_voo; // nenhum problema → pula os outros loops
                }


            }
                
        }
       
        for (int j = 0; j < N; j++) //Colisão crítica e longe da pista de pouso...
        {
            if (j == voo_atual) continue;
            if (frota[j].status != STATUS_VOANDO) continue;

            float dx = frota[voo_atual].x - frota[j].x;
            float dy = frota[voo_atual].y - frota[j].y;
            float dist2 = dx * dx + dy * dy;

            

            if (dist2 < 0.01f)
            {
                printf("Colisão crítica entre %d e %d. Matando %d.\n", voo_atual, j, voo_atual);
                kill(frota[voo_atual].pid, SIGKILL);
                frota[voo_atual].status = STATUS_REMOVIDO;
                voos_ativos--;
                goto proximo_voo;
                //Depois pensar em prioridade
            }
        }


        for (int j = 0; j < N; j++) //Colisão leve apenas frear
        {
        if (j == voo_atual) continue;
        if (frota[j].status != STATUS_VOANDO) continue;

        float dx = frota[voo_atual].x - frota[j].x;
        float dy = frota[voo_atual].y - frota[j].y;
        float dist2 = dx * dx + dy * dy;

        float dx_pista_atual = frota[voo_atual].x - 0.5f;
        float dy_pista_atual = frota[voo_atual].y - 0.5f;
        float dist_pista_atual2 = dx_pista_atual * dx_pista_atual + dy_pista_atual * dy_pista_atual;
        
        float dx_pista_j = frota[j].x - 0.5f;
        float dy_pista_j = frota[j].y - 0.5f;
        float dist_pista_j2 = dx_pista_j * dx_pista_j + dy_pista_j * dy_pista_j;
        
        
        if (0.01f < dist2 < 0.04f) 
        {
            //checando quem possui maior prioridade:            
            printf("Proximidade entre %d e %d. Freando %d.\n", voo_atual, j, voo_atual);
            kill(frota[voo_atual].pid, SIGUSR2);
            goto proximo_voo;
            
        }
        }



            printf("While 1 do pai\n");
            // Acorda
            kill(frota[voo_atual].pid, SIGCONT);
            sleep(2); // tempo de "voo", dando 2 segundos pro filho voar
            kill(frota[voo_atual].pid, SIGSTOP);
            
            printf("Local do aviao %d: %f %f\n",voo_atual,frota[voo_atual].x, frota[voo_atual].y);
            // Verifica se pousou
            if (frota[voo_atual].x == 0.5f && frota[voo_atual].y == 0.5f) {
                printf("Controlador: avião PID %d pousou.\n", frota[voo_atual].pid);
                frota[voo_atual].status = STATUS_ATERRISSADO;
                voos_ativos--;
            }
    }
        else if (frota[voo_atual].status == STATUS_ATERRISSADO || frota[voo_atual].status == STATUS_REMOVIDO) {
            // Já não está voando, ignora
            // printf("Avião %d já finalizado\n", atual);
        }
        else if (frota[voo_atual].status == STATUS_PAUSADO){
            for (int j = 0; j < N; j++) //Colisão crítica e longe da pista de pouso...
        {
            if (j == voo_atual) continue;
            if (frota[j].status != STATUS_VOANDO) continue;

            float dx = frota[voo_atual].x - frota[j].x;
            float dy = frota[voo_atual].y - frota[j].y;
            float dist2 = dx * dx + dy * dy;
            
            float dx_pista_atual = frota[voo_atual].x - 0.5f;
            float dy_pista_atual = frota[voo_atual].y - 0.5f;
            float dist_pista_atual2 = dx_pista_atual * dx_pista_atual + dy_pista_atual * dy_pista_atual;
            
            float dx_pista_j = frota[j].x - 0.5f;
            float dy_pista_j = frota[j].y - 0.5f;
            float dist_pista_j2 = dx_pista_j * dx_pista_j + dy_pista_j * dy_pista_j;


            if (0.04f < dist2 < 0.01f)//Se o aviao esta em uma distancia entre 0.2 e 0.1 de outros avioes
            {
                printf("O aviao %d ainda esta proximo de %d. Espere!\n", voo_atual, j);
                if (dist_pista_atual2 < dist_pista_j2){//Prioriza quem estiver mais perto da pista!
                    printf("Libera o atual!\n");
                    kill(frota[voo_atual].pid, SIGUSR2);
                }
                else{
                    printf("Mantenha-se freando! Ha um aviao a sua frente.\n");
                    goto proximo_voo;
                }
            }
        }

        }
        proximo_voo:
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