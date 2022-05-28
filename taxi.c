#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>  
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h> 
#include "lib_simulazione.h"

  int occupato = 0;
  int posx = 0, posy = 0;
  struct shared_memory * sm;

  void handle_signal(int signal); /* signal handler */  


  int main(){

      int k = 0 ,l = 0,m = 0,n = 0, cap = 0, y = 0, z = 0, i=0, ind =0, id_l =0, tempo = 0, aux2 = 0;
      char message[120];
      char aux[120]; 
      int destx = 0, desty = 0, sm_id = 0, sem_id = 0, vSem_id = 0, so_cap = 0, msg_id = 0;
      struct sigaction sa;
      struct timespec so_timesec2;
      int so_taxi = 0, so_sources = 0, so_holes = 0, so_top_cells = 0, so_cap_min = 0, so_cap_max = 0, so_timesec_min = 0;
      int so_timesec_max = 0, so_timeout = 0, so_duration = 0;
      int pipeIJ[2], pipeC[2];
      FILE * myfileM, * client, * ftime, *fconf;
      struct sembuf sem, vSem;
      struct msgbuff msg;
      int num_byte, clienti;

      pipe(pipeIJ); 
      pipe(pipeC);
      myfileM = fopen("var.txt","r");
      client = fopen("num_clienti.txt","a");
      ftime = fopen("time.txt","a");


       fconf = fopen("configurazione.txt","r");
       fscanf(fconf,"%d",&so_taxi);
       fscanf(fconf,"%d",&so_sources);
       fscanf(fconf,"%d",&so_holes);
       fscanf(fconf,"%d",&so_top_cells);
       fscanf(fconf,"%d",&so_cap_min);
       fscanf(fconf,"%d",&so_cap_max);
       fscanf(fconf,"%d",&so_timesec_min);
       fscanf(fconf,"%d",&so_timesec_max);
       fscanf(fconf,"%d",&so_timeout);
       fscanf(fconf,"%d",&so_duration);
       fclose(fconf);
      
     

      /*fgets(aux, 7, myfileM);
      sm_id = atoi(aux);*/
      fscanf(myfileM,"%d",&sm_id);
     

      fscanf(myfileM,"%d",&sem_id);
     

      fscanf(myfileM,"%d",&vSem_id);
    

      fscanf(myfileM,"%d",&so_cap);
     

      fscanf(myfileM,"%d",&msg_id);
     

      fscanf(myfileM,"%d",&z);
   


       /*sem_id = fgetc(myfileM);
       while(sem_id == '\n' || sem_id == ' ')
                sem_id = fgetc(myfileM); 

       vSem_id = fgetc(myfileM);
        while(vSem_id == '\n' || vSem_id == ' ')
                vSem_id = fgetc(myfileM); 
       so_cap = fgetc(myfileM);
       while(so_cap == '\n' || so_cap == ' ')
                so_cap = fgetc(myfileM);

      msg_id = fgetc(myfileM);
       while(msg_id == '\n' || msg_id == ' ')
                msg_id = fgetc(myfileM);  

      z = fgetc(myfileM);
       while(z == '\n' || z == ' ')
                z = fgetc(myfileM);    */         

       fclose(myfileM);       

      
      
      /*sm_id = sm_id - '0';
      sem_id = sem_id - '0';
      vSem_id = vSem_id - '0';
      so_cap = so_cap - '0';
      msg_id = msg_id - '0';
      z = z - '0';*/

      so_timesec2.tv_nsec = z;
      so_timesec2.tv_sec = 0;
      /*sm_id = shmget(MY_KEY_SM, sizeof(struct shared_memory) ,0);*/
      sm = shmat(sm_id,NULL,0);
    /*  sem_id = semget(MY_KEY_SEM1,3, 0);/*init di 2 semafori  */
      /*vSem_id = semget(MY_KEY_SEM2,(SO_WIDTH*SO_HEIGHT),0);
      msg_id = msgget(MY_KEY, 0);*/
      
   
      aux[0] = '\0';

         srand(time(NULL) ^ getpid()); /* inizializza il rand con l'ora del pc, diverso per ogni processo*/
            /*
             * CODICE PROCESSO FIGLIO ---> creazione di un nuovo taxi
             * 
             */
        close(pipeIJ[0]);
        close(pipeC[0]);

        
         
          ROSSO_TAXI;
          id_l = sm->id;
          sm->id++;
            do{
               k = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
               l = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
               printf("excv");
             
               
            } while((sm->mappa[k][l] == -1) || (sm->mappa[k][l] >= so_cap));
            sm->mappa[k][l]++;
          VERDE_TAXI; 
            
         
           
                /* scrittura degli indici nella pipe*/
               posx = k;
               posy = l;
              


               sem.sem_num = (posx * SO_WIDTH) + posy;     
               sem.sem_op = -1;      
  


               
             

                ROSSO_TAXI;
                num_byte = msgrcv(msg_id, &msg, sizeof(msg), 2, 0);
             
                aux2 = sm->index2;
                

             
                for(y=0;y<strlen(msg.dest);y++){
                  message[y] = msg.dest[y];
                }
              
                

                for(y=0,z=0;y<strlen(message);y++){
                  if(message[y] >= '0' && message[y] <= '9'){
                     strncat(aux,&message[y],1);
                  }else if(message[y] == 'c'){
                    if(z < 4 && z == 0){

                      sm->vDestX[aux2] = atoi(aux);
              
                      aux[0] = '\0'; /*reset della stringa*/

                    } else if(z < 4 && z == 1){

                      sm->vDestY[aux2] = atoi(aux);
         
                      aux[0] = '\0'; /*reset della stringa*/

                    } else if(z < 4 && z == 2){
                      
                      sm->vSourcesX[aux2] = atoi(aux);
                
                      aux[0] = '\0'; /*reset della stringa*/

                    } else if(z < 4 && z == 3){
                      sm->vSourcesY[aux2] = atoi(aux);
                      
           
                      aux[0] = '\0'; /*reset della stringa*/
                      sm->index2++;
                      break;
                    }

                      z++;
                      
                  }
                }
                

              
                VERDE_TAXI;
                  
                

                

                bzero(&sa, sizeof(sa));  /* set all bytes to zero */
 
                sa.sa_handler = handle_signal;

               /* Now, setting the handler */
                sigaction(SIGALRM, &sa, NULL);

               while(1){
             

                
                ROSSO_TAXI;
                  if((occupato==0) && (sm->vSourcesX[sm->index] == posx)&&(sm->vSourcesY[sm->index] == posy)){
                        
                     
                        sm->inactive = 0;
                      
                        destx = sm->vDestX[sm->index];
                        desty = sm->vDestY[sm->index];
                      
                        sm->viaggi++;
                        sm->index++;
                        occupato = 1;
                       
                        clienti++;
                        dprintf(pipeC[1],"%d\n", id_l);
                        dprintf(pipeC[1],"%d\n", clienti);
                        
                      }else{
                        sm->inactive++;
                      }
                      if(sm->inactive > 1000){
                        
                    
                        sm->index++;
                        sm->inactive = 0;
                        
                      }
                      VERDE_TAXI;
                      
                  
                  if(occupato == 0 && sm->richieste[posx][posy] == 0){
                    /*ricerca so_sources adiacenti*/
                 
                    if((posx != SO_WIDTH-1)&&(sm->richieste[posx+1][posy] == 1)){ /*spostamento in alto */
                     /*posx++*/
                       cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                       while(abs(cap) < 0 ){
                          vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);
                          cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                        }
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                        vSem.sem_op = 1;
                        semop(vSem_id,&vSem,1);
                        sm->mappa[posx][posy]--;
                        tempo = tempo + (so_timeout - alarm(so_timeout));
                        sm->top_cells[((posx) * SO_WIDTH) + posy];
                        posx++;
                        sm->mappa[posx][posy]++;
                        nanosleep(&so_timesec2, NULL);
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                        vSem.sem_op = -1;
                        semop(vSem_id,&vSem,1); 

                    }else if((posx != 0)&&(sm->richieste[posx-1][posy] == 1)){/*spostamento in basso */
                        /*posx--*/
                        cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                        while(abs(cap) < 0 ){
                              vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                              cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                            }
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                        vSem.sem_op = 1;
                        semop(vSem_id,&vSem,1);
                        sm->mappa[posx][posy]--;
                        tempo = tempo + (so_timeout - alarm(so_timeout));
                        sm->top_cells[((posx) * SO_WIDTH) + posy];
                        posx--;
                        sm->mappa[posx][posy]++;
                        nanosleep(&so_timesec2, NULL);
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                        vSem.sem_op = -1;
                        semop(vSem_id,&vSem,1);


                    }else if((posy != SO_HEIGHT-1)&&(sm->richieste[posx][posy+1] == 1)){/*spostamento a destra*/
                        /*posy++*/
                        cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                        while(abs(cap) < 0 ){
                              vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                              cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                            }
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                        vSem.sem_op = 1;
                        semop(vSem_id,&vSem,1);
                        sm->mappa[posx][posy]--;
                        tempo = tempo + (so_timeout - alarm(so_timeout));
                        sm->top_cells[((posx) * SO_WIDTH) + posy];
                        posy++;
                        sm->mappa[posx][posy]++;
                        nanosleep(&so_timesec2, NULL);
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                        vSem.sem_op = -1;
                        semop(vSem_id,&vSem,1);

                    }else if((posy != 0)&&(sm->richieste[posx][posy-1] == 1)){ /*spostamento a sinistra*/
                        /*posy--*/
                        cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                        while(abs(cap) < 0 ){
                              vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                              cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                            }
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                        vSem.sem_op = 1;
                        semop(vSem_id,&vSem,1);
                        sm->mappa[posx][posy]--;
                        tempo = tempo + (so_timeout - alarm(so_timeout));
                        sm->top_cells[((posx) * SO_WIDTH) + posy];
                        posy--;
                        sm->mappa[posx][posy]++;
                        nanosleep(&so_timesec2, NULL);
                        vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                        vSem.sem_op = -1;
                        semop(vSem_id,&vSem,1);

                    }else{
                      
                      z = rand() % (3 + 1 - 0) + 0;
                      switch(z){
                        case 0: /*spostamento in alto */
                          if(posx != SO_WIDTH-1 && sm->mappa[posx+1][posy] != -1){
                            /*posx++*/
                             cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                             while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                              }
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                              vSem.sem_op = 1;
                              semop(vSem_id,&vSem,1);
                              sm->mappa[posx][posy]--;
                              tempo = tempo + (so_timeout - alarm(so_timeout));
                              sm->top_cells[((posx) * SO_WIDTH) + posy];
                              posx++;
                              sm->mappa[posx][posy]++;
                              nanosleep(&so_timesec2, NULL);
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);

                          }
                        break;
                        case 1: /*spostamento in basso */
                          if(posx != 0 && sm->mappa[posx-1][posy] != -1){
                             /*posx--*/
                              cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                              while(abs(cap) < 0 ){
                                    vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                    vSem.sem_op = -1;
                                    semop(vSem_id,&vSem,1);
                                    cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                                  }
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                              vSem.sem_op = 1;
                              semop(vSem_id,&vSem,1);
                              sm->mappa[posx][posy]--;
                              tempo = tempo + (so_timeout - alarm(so_timeout));
                              sm->top_cells[((posx) * SO_WIDTH) + posy];
                              posx--;
                              sm->mappa[posx][posy]++;
                              nanosleep(&so_timesec2, NULL);
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                            
                          }
                        break;
                        case 2: /*spostamento a destra */
                          if(posy != SO_HEIGHT-1 && sm->mappa[posx][posy+1] != -1){
                              /*posy++*/
                              cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                              while(abs(cap) < 0 ){
                                    vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                    vSem.sem_op = -1;
                                    semop(vSem_id,&vSem,1);
                                    cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                                  }
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                              vSem.sem_op = 1;
                              semop(vSem_id,&vSem,1);
                              sm->mappa[posx][posy]--;
                              tempo = tempo + (so_timeout - alarm(so_timeout));
                              sm->top_cells[((posx) * SO_WIDTH) + posy];
                              posy++;
                              sm->mappa[posx][posy]++;
                              nanosleep(&so_timesec2, NULL);
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                          }

                        break;
                        case 3: /*spostamento a sinistra */
                          if(posy != 0 && sm->mappa[posx][posy-1] != -1){
                              /*posy--*/
                              cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                              while(abs(cap) < 0 ){
                                    vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                    vSem.sem_op = -1;
                                    semop(vSem_id,&vSem,1);
                                    cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                                  }
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                              vSem.sem_op = 1;
                              semop(vSem_id,&vSem,1);
                              sm->mappa[posx][posy]--;
                              tempo = tempo + (so_timeout - alarm(so_timeout));
                              sm->top_cells[((posx) * SO_WIDTH) + posy];
                              posy--;
                              sm->mappa[posx][posy]++;
                              nanosleep(&so_timesec2, NULL);
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                          }

                        break;
                      }
                    }

                  }

                  if(occupato == 1){
                    /*sezione movimento taxi*/
                
                      if(posx < destx){
                        if(sm->mappa[posx+1][posy] != -1){
                           /*posx++*/
                             cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                             while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                              }
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                              vSem.sem_op = 1;
                              semop(vSem_id,&vSem,1);
                              sm->mappa[posx][posy]--;
                              tempo = tempo + (so_timeout - alarm(so_timeout));
                              sm->top_cells[((posx) * SO_WIDTH) + posy];
                              posx++;
                              sm->mappa[posx][posy]++;
                              nanosleep(&so_timesec2, NULL);
                              vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                              vSem.sem_op = -1;
                              semop(vSem_id,&vSem,1);
                              /*decremento del getvalue della cella precedente cellaprecedente--*/
                            /* if(abs(cap) > 0){
                              incrementa la cella corrente
                                posx++;
                              decrementare la cella seguente

                             }
                             else{
                                vSem.sem_num = ((posx+1) * SO_WIDTH) + posy
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                             } */
                        }else if(posy+1 == SO_HEIGHT){
                          /*posy--*/
                          cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posy--;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

                          /*posx--*/
                          cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx--;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);
                        }else{
                          /*posy++*/
                          cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posy++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

              /*posx--*/
                          cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx--;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);                          
                        }
                      }else if(posx > destx){
                        if(sm->mappa[posx-1][posy] != -1){
                          /*posx--*/
                          cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx--;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);
                        }else if(posy+1 == SO_WIDTH){
                          /*posy--*/
                          cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posy--;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

                          /*posx++*/
                         cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                         while(abs(cap) < 0 ){
                            vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                            cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                          }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);
                        }else{
                         /*posy++*/
                          cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                          while(abs(cap) < 0 ){
                                vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                vSem.sem_op = -1;
                                semop(vSem_id,&vSem,1);
                                cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                              }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posy++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

                          /*posx++*/
                         cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                         while(abs(cap) < 0 ){
                            vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                            cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                          }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);
                        }
                      }else if(posx == destx){
                        if(posy < desty){
                          if(sm->mappa[posx][posy+1] != -1){
                            /*posy++*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy++;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }else if(posy+1 == SO_WIDTH){
                            /*posx--*/
                            cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posx--;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);

                            /*posy++*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy++;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }else{
                             /*posx++*/
                         cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                         while(abs(cap) < 0 ){
                            vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                            cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                          }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

                            /*posy++*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy+1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy+1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy++;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }
                        }else if(posy > desty){
                          if(sm->mappa[posx][posy-1] != -1){
                            /*posy--*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy--;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }else if(posx == 0){
                            /*posx++*/
                         cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL);
                         while(abs(cap) < 0 ){
                            vSem.sem_num = ((posx+1) * SO_WIDTH) + posy;
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                            cap = semctl(vSem_id,(((posx+1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                          }
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                          vSem.sem_op = 1;
                          semop(vSem_id,&vSem,1);
                          sm->mappa[posx][posy]--;
                          tempo = tempo + (so_timeout - alarm(so_timeout));
                          sm->top_cells[((posx) * SO_WIDTH) + posy];
                          posx++;
                          sm->mappa[posx][posy]++;
                          nanosleep(&so_timesec2, NULL);
                          vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                          vSem.sem_op = -1;
                          semop(vSem_id,&vSem,1);

                            /*posy--*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy--;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }else{
                            /*posx--*/
                            cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx-1) * SO_WIDTH) + posy;
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx-1) * SO_WIDTH) + posy),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posx--;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);

                            /*posy--*/
                            cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL);
                            while(abs(cap) < 0 ){
                                  vSem.sem_num = ((posx) * SO_WIDTH) + (posy-1);
                                  vSem.sem_op = -1;
                                  semop(vSem_id,&vSem,1);
                                  cap = semctl(vSem_id,(((posx) * SO_WIDTH) + (posy-1)),GETVAL); /*aggiornare il valore di cap*/
                                }
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*incremento della cella corrente (semaforo)*/
                            vSem.sem_op = 1;
                            semop(vSem_id,&vSem,1);
                            sm->mappa[posx][posy]--;
                            tempo = tempo + (so_timeout - alarm(so_timeout));
                            sm->top_cells[((posx) * SO_WIDTH) + posy];
                            posy--;
                            sm->mappa[posx][posy]++;
                            nanosleep(&so_timesec2, NULL);
                            vSem.sem_num = ((posx) * SO_WIDTH) + posy; /*decremento della cella corrente (semaforo)*/
                            vSem.sem_op = -1;
                            semop(vSem_id,&vSem,1);
                          }
                        }else if(posy == desty){
                          /*arrivo destinazione*/
                          dprintf(pipeIJ[1],"%d \n", id_l);
                          dprintf(pipeIJ[1],"%d \n", tempo);
                          tempo = 0;
                          occupato = 0;
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
                         
                          sm->successi++;
                          break;
                        }
                      }
                    } 
                    
              }

            
              
             
            
            exit(EXIT_SUCCESS);

            close(pipeIJ[1]);
            close(pipeC[1]);

  }



  void handle_signal(int signum){
    

    char * args[] = {"./taxi",NULL}; 

    switch (signum) {
    case SIGINT:
     
    /* handler SIGINT */
    break;
    case SIGALRM:
      if(occupato == 0){
     
        sm->mappa[posx][posy]--;
        execv(args[0],args);
      }else{
        sm->count_abort++;
        sm->mappa[posx][posy]--;
       
        execv(args[0],args);
      }
      
 
    break;
     case SIGUSR1:
      /*sm->flag = 1;*/
      printf("segnale madato al master\n");
     break;

     
  }
  
}
