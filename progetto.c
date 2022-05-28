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



void handle_signal(int signal); /* signal handler */         
void master_handler_signal(int signal); /* signal handler */     
void handler_signal_cmd(int signal);
void handler_stampa(int signal);
/*pid_t vChild[SO_SOURCES+SO_TAXI] = {0};*/



struct shared_memory * sm;
int occupato = 0;
int posx = 0,posy = 0;
int k = 0,l = 0, msg_id;
struct msgbuff msg;
pid_t vChild[1000] = {0};
int so_taxi=0, so_sources=0;

int main(){
   pid_t childpid;
   int so_cap = 0, id_l=0, tempo = 0; /* capienza di taxi per ogni cella*/
   int status, clienti = 0;
   int matrix[SO_WIDTH][SO_HEIGHT] = {0};
   char buf[READ_BUF_SIZE];
   char bufR[READ_BUF_SIZE], bufM[READ_BUF_SIZE], bufT[READ_BUF_SIZE];
   int mRichieste[SO_WIDTH][SO_HEIGHT] = {0};
   int i = 0, n = 0, m = 0;
   int j = 0, y =0, z=0; /* j indice di vChild*/
   int sm_id;
   int sec, num_byte = -1;
   int out_fd; /* fd gen_taxi*/
   int out_fd_sources; /*fd gen_sources*/
   int out_fd_dest_master, out_fd_clienti; 
   struct sigaction sa, sa_master, sa_cmd, sa_stp;
   struct sembuf sem, vSem;
   struct msqid_ds ds;
   /*struct msgbuff msg;*/
   int vTime[5000] = {0}; /*[so_taxi]*/
   int vClienti[5000] = {0};
   int so_duration = 0;
   pid_t val_fork = 0;
   int new_fd, aux2 = 0;
   int new_fd2, new_fd3, new_fd4;
   int saved_stdout;
   ssize_t num_bytes = 0, num_bytes2 = 0, num_bytes3 = 0, num_bytes4;
   int tempx = 0, templ,tempk;
   int tempy = 0, cap=0;
   int destx = 0, desty=0;
   int sem_id, vSem_id;
   FILE * myfileT, * myfileM, * client, *ftime, * fconf;
   char message[120], aux[120], str[12];
   int so_holes = 0, so_cap_min = 0, so_cap_max = 0, so_timesec_min = 0,so_timesec_max = 0;
   int so_timeout = 0, so_top_cells = 0;
   int pipeIJ[2]; /* vettore ospite della PIPE di grandezza 2*/
   int pipeR[2]; /*pipe per le richieste */
   int pipeC[2];
   
   int ind;
   struct timespec so_timesec = {3,125}, so_timesec2; /*1,0 == 0,1000*/
   pipe(pipeIJ); /* inizializzazione della pipe, creazione dei due file descriptors (read/write)*/
   pipe(pipeR);
   pipe(pipeC);
   

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
   

   
   
   ftime = fopen("time.txt","w"); 
   client = fopen("num_clienti.txt","w");
   sem_id = semget(IPC_PRIVATE,3, 0600);/*init di 2 semafori  */
   vSem_id = semget(IPC_PRIVATE,(SO_WIDTH*SO_HEIGHT),0600);
   sm_id = shmget(IPC_PRIVATE, sizeof(struct shared_memory) ,0600);
   semctl(sem_id,1,SETVAL,1);2
   semctl(sem_id,2,SETVAL,1);
   semctl(sem_id,3,SETVAL,1);
   semctl(sem_id,4,SETVAL,1);
   sm = shmat(sm_id,NULL,0);

 
   /*inizializzazione a 1 del semaforo utilizzato per le celle della mappa*/
   for(z=1;z<=(SO_WIDTH*SO_HEIGHT);z++){
      semctl(vSem_id,z,SETVAL,so_cap); 
   }
  
   
   msg_id = msgget(MY_KEY, IPC_CREAT | 0600);
   /*msg.mtype = 1;*/
     ind = 0;

   shmctl(sm_id, IPC_RMID, NULL); 
   srand(time(NULL) ^ getpid());
   z = rand() % (so_timesec_max + 1 - so_timesec_min) + so_timesec_min;
   so_timesec2.tv_nsec = z; /* z per 100 per valori piccoli di z*/
   so_timesec2.tv_sec = 0;
   if(so_cap_min == so_cap_max){
    so_cap = so_cap_min;
   } else{
       so_cap = rand() % (so_cap_max + 1 - so_cap_min) + so_cap_min;
   }
  

   myfileT = fopen("var.txt","w");
   fprintf(myfileT,"%d\n",sm_id);
   fprintf(myfileT,"%d\n",sem_id);
   fprintf(myfileT,"%d\n",vSem_id);
   fprintf(myfileT,"%d\n",so_cap);
   fprintf(myfileT,"%d\n",msg_id);
   fprintf(myfileT,"%d",z);
   fclose(myfileT);



    RESET_DEST();
    inizializzazione_mappa_richieste();
    generazione_holes(so_holes);
    


    k=0;
    l=0;
    j=0;


   sm->flag = 0;
   sm->testStop = 0;
   sm->scount = 0;
   sm->index = 0;
   sm->count_abort = 0;
   sm->successi = 0;
   sm->viaggi = 0;
   sm->id = 0;
   sm->inactive = 0;
   sm->index2 = 0;

    for(z=0;z<(SO_WIDTH*SO_HEIGHT);z++){
       sm->top_cells[z] = 0;
   }
   


   

    bzero(&sa_cmd, sizeof(sa_cmd));  /* set all bytes to zero */
 
    sa_cmd.sa_handler = handler_signal_cmd;

    



/* generazione dei processi taxi*/
   for (i=0; i<so_taxi; i++) {
      switch (vChild[j] = fork()){
      case -1:
         /* Handle error */
         printf("-> errore durante la generazione dei taxi");
         exit(EXIT_FAILURE);
      case 0:
        
         srand(time(NULL) ^ getpid()); /* inizializza il rand con l'ora del pc, diverso per ogni processo*/
            /*
             * CODICE PROCESSO FIGLIO ---> creazione di un nuovo taxi
             * 
             */
         close(pipeIJ[0]); /*chiusura del file descriptor di lettura*/
         close(pipeC[0]);
         
          ROSSO_TAXI;
           id_l = sm->id;
           sm->id++;
            do{
               k = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
               l = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
            
               
            } while((sm->mappa[k][l] == -1) || (sm->mappa[k][l] >= so_cap));
            sm->mappa[k][l]++;
          VERDE_TAXI  
            
          

           
                /* scrittura degli indici nella pipe*/
               posx = k;
               posy = l;
        


               sem.sem_num = (posx * SO_WIDTH) + posy;     
               sem.sem_op = -1;      
  


               ROSSO_TAXI;
               sm->tcount++;
             
               VERDE_TAXI;

               while(1){
                
                if((sm->tcount == so_taxi)&&(sm->scount == so_sources)){
                
                   break;
                }
                
               }

               
             

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
                        
                       /* if(sm->index > 1000){
                          sm->index = 0;
                        }*/
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
                        ind = (posx * SO_WIDTH) + posy;
                        sm->top_cells[ind]++;
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
                        ind = (posx * SO_WIDTH) + posy;
                        sm->top_cells[ind]++;
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
                        ind = (posx * SO_WIDTH) + posy;
                        sm->top_cells[ind]++;
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
                        ind = (posx * SO_WIDTH) + posy;
                        sm->top_cells[ind]++;
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
                              ind = (posx * SO_WIDTH) + posy;
                              sm->top_cells[ind]++;
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
                              ind = (posx * SO_WIDTH) + posy;
                              sm->top_cells[ind]++;
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
                              ind = (posx * SO_WIDTH) + posy;
                              sm->top_cells[ind]++;
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
                              ind = (posx * SO_WIDTH) + posy;
                              sm->top_cells[ind]++;
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
                              ind = (posx * SO_WIDTH) + posy;
                              sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                          ind = (posx * SO_WIDTH) + posy;
                          sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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
                            ind = (posx * SO_WIDTH) + posy;
                            sm->top_cells[ind]++;
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

             


            close(pipeIJ[1]);
            close(pipeC[1]);
            
            exit(EXIT_SUCCESS);
            break;
         
      default:

         break;
      }
      j++;
   }
  



   /*generazione delle richieste */

   for(i = 0;i<so_sources;i++){
      switch(vChild[j] = fork()){
        case -1:
          printf("-> errore durante la generazione della richiesta");
          exit(EXIT_FAILURE);
        break;
        case 0:

          sigaction(SIGUSR1, &sa_cmd, NULL);
          srand(time(NULL) ^ getpid()); /* inizializza il rand con l'ora del pc, diverso per ogni processo*/
            /*
             * CODICE PROCESSO FIGLIO ---> creazione di una nuova richiesta
             * 
             */
         close(pipeR[0]); /*chiusura del file descriptor di lettura*/
         
        ROSSO_RICHIESTE;

            do{
               k = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
               l = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
               
            } while((sm->richieste[k][l] == -1) || (sm->richieste[k][l] == 1));
              sm->richieste[k][l]++;
        VERDE_RICHIESTE; 
            
          
           
               dprintf(pipeR[1],"%d \n", k);
               dprintf(pipeR[1],"%d \n", l); /* scrittura degli indici nella pipe*/
               
          


              ROSSO_DEST
               sm->scount++;
               VERDE_DEST;

               while(1){
                
                 if(sm->scount == so_sources){
                
                      
                      break;
                      
                  }else{
                   
                  }
                

              }


           while(1){
              ROSSO_DEST;  
                do{
                 
                     n = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
                     m = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
                         
                }while((sm->mappa[n][m] == -1)||(n == k && m == l));
           
                
                 msg.mtype = 2;
                 sprintf(msg.dest,"%dc%dc%dc%dc",n,m,k,l);
                 msgsnd(msg_id, &msg, sizeof(msg), 0);

                
                  
                 
              
              VERDE_DEST;
             
             nanosleep(&so_timesec, NULL);

          }
            
            
            close(pipeR[1]);
            
            exit(EXIT_SUCCESS);
        break;
        default:

        break;
      }
      j++;

   }

    bzero(&sa_master, sizeof(sa_master));  
    sa_master.sa_handler = master_handler_signal;
    sigaction(SIGALRM, &sa_master, NULL);


     bzero(&sa_stp, sizeof(sa_stp));  
    sa_stp.sa_handler = handler_stampa;
    sigaction(SIGUSR2, &sa_stp, NULL);
   
   
    while((sm->tcount != so_taxi)&&(sm->scount != so_sources)){
     /*printf("In attesa dell'inizializzazione...\n");*/
    }
   
    
    alarm(so_duration);
    printf("Simulazione iniziata\n");
    kill(getpid(),SIGUSR2);


   

   

   

   while ((childpid = wait(&status)) != -1) { /*attesa della terminazione di tutti i processi figli*/
      
   }
   fclose(client);
   fclose(ftime);
   semctl(vSem_id, 0,IPC_RMID);
   semctl(sem_id, 0,IPC_RMID);



   close(pipeIJ[1]); /* chiusura del file descriptor di scrittura*/
   close(pipeR[1]);
   close(pipeC[1]);

  /* buf = malloc(READ_BUF_SIZE);*/
   if ((out_fd = open(MY_OUT_TAXI,
            O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR)) == -1) {
      fprintf(stderr, "-> errore: %d (%s) durante l'aperture del file \"%s\"\n", errno,strerror(errno), MY_OUT_TAXI);
      exit(EXIT_FAILURE);
   }

    if ((out_fd_sources = open(MY_OUT_SOURCES,
            O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR)) == -1) {
      fprintf(stderr, "-> errore: %d (%s) durante l'aperture del file \"%s\"\n", errno,strerror(errno), MY_OUT_SOURCES);
      exit(EXIT_FAILURE);
   }

     if ((out_fd_clienti = open(MY_OUT_CLIENTI,
            O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR)) == -1) {
      fprintf(stderr, "-> errore: %d (%s) durante l'aperture del file \"%s\"\n", errno,strerror(errno), MY_OUT_CLIENTI);
      exit(EXIT_FAILURE);
   }



   saved_stdout = dup(1); /* backup del file descriptor stdout */
   new_fd = dup2(out_fd,STDOUT_FILENO); /*cambiamento del file descriptor*/

  while (num_bytes = read(pipeIJ[0], buf, READ_BUF_SIZE)) {
 
     write(STDOUT_FILENO, buf, num_bytes); /* stampa della pipe*/
  
      
   }

   dup2(saved_stdout, STDOUT_FILENO);  /*ripristino del file descriptor */
   

   myfileM = fopen(MY_OUT_TAXI, "r");
    while(fscanf(myfileM,"%d",&tempx) != EOF){
      fscanf(myfileM,"%d",&tempy);
       vTime[tempx] = tempy;
    }
    fclose(myfileM);

   saved_stdout = dup(1); /* backup del file descriptor stdout */
   new_fd2 = dup2(out_fd_sources, STDOUT_FILENO);

   while (num_bytes2 = read(pipeR[0], bufR, READ_BUF_SIZE)) {
 
     write(STDOUT_FILENO, bufR, num_bytes2); /* stampa della pipe*/
 
   }

   dup2(saved_stdout, STDOUT_FILENO);
   saved_stdout = dup(1);
   new_fd3 = dup2(out_fd_clienti, STDOUT_FILENO);

    while (num_bytes3 = read(pipeC[0], bufT, READ_BUF_SIZE)) {
 
     write(STDOUT_FILENO, bufT, num_bytes3); /* stampa della pipe*/
 
      
   }

   dup2(saved_stdout, STDOUT_FILENO);

   
     myfileM = fopen(MY_OUT_CLIENTI, "r");
    while(fscanf(myfileM,"%d",&tempk) != EOF){
      fscanf(myfileM,"%d",&templ);
       vClienti[tempk] = templ;
    }
    fclose(myfileM);


  close(saved_stdout);
    close(pipeR[0]);
   close(pipeIJ[0]);
   close(pipeC[0]);
   

   dup2(saved_stdout, STDOUT_FILENO);  /*ripristino del file descriptor */
   close(saved_stdout);
  

   client = fopen("num_clienti.txt", "r");


   

   /*max taxi richieste prese in carico*/
   l=vClienti[0];
   k=0;
   for(z=1;z<(sizeof(vClienti)/sizeof(vClienti[0]));z++){
    if(l < vClienti[z]){
     
      l = vClienti[z];
      k = z;
    }
   }

   /*max taxi viaggio piu lungo*/
   n = vTime[0];
   m = 0;
   for(z=1;z<(sizeof(vTime)/sizeof(vTime[0]));z++){
    if(n < vTime[z]){
     
      n = vTime[z];
      m = z;
    }
   }

  printf("___STATISTICHE______\n");
  printf("\n"); 
   if(sm->successi > 0){
      printf("IL TAXI %d HA RACCOLTO %d RICHIESTE.\n",k,vClienti[k]);
      printf("IL TAXI %d HA PERCORSO %d SECONDI (eventuali millisecondi persi)\n",m,vTime[m]);
   }



    if (msgctl(msg_id, IPC_STAT, &ds) == -1) 
    {
      perror ("-> errore ipc_stat");
    }
    


 
   printf("VIAGGI TOTALI: %d\n", sm->viaggi);
   printf("VIAGGI CON SUCCESSO: %d \n",sm->successi);
   printf("VIAGGI ABORTED: %d\n",sm->count_abort);
   printf("VIAGGI INEVASI: %ld\n",(sm->viaggi - (sm->successi + sm->count_abort)) + ds.msg_qnum);
  
   while(so_top_cells > 0){
    y = sm->top_cells[0];
    ind = 0;
    for(z = 1; z < (SO_WIDTH*SO_HEIGHT);z++){
      if(y < sm->top_cells[z]){
        y = sm->top_cells[z];
        ind = z;
      }
    }
    so_top_cells--;
    printf("TOP CELL(%d,%d):  %d\n",(ind/SO_HEIGHT),(ind%SO_WIDTH),y);
    sm->top_cells[ind] = 0;
   }
   
       
    msgctl(msg_id, IPC_RMID, NULL); 
  
   
   
   printf("_______________________________________________\n");
   printf("MATRICE SO_SOURCES: \n");
   /*STAMPA_MATRIX_MRICHIESTE()*/
   STAMPA_RICHIESTE_CONDIVISA();
   printf("%ld,%ld\n",so_timesec.tv_sec,so_timesec.tv_nsec);
   /*fclose(ftime);*/
   fclose(client);
   
}

void handle_signal(int signum){
  
  char * args[] = {"./taxi",NULL}; 

    switch (signum) {
    case SIGINT:
      printf("(%d) ricevuto segnale: sigint\n", getpid());
    /* handler SIGINT */
    break;
    case SIGALRM:
      if(occupato == 0){
        
        sm->mappa[posx][posy]--;

        kill(getpid(), 0);
        if(errno!=ESRCH)
          execv(args[0],args);
        
        
      }else{
        sm->count_abort++;
        sm->mappa[posx][posy]--;
       

        kill(getpid(), 0);
        if(errno!=ESRCH)
          execv(args[0],args);
      }
      
 
    break;
     case SIGUSR1:
      /*sm->flag = 1;*/
      printf("USER DEFINED RICEVUTO\n");
     break;
     
  }
  
}

void master_handler_signal(int signum){
  int i;
  switch (signum) {
    case SIGINT:
      printf("(%d) ricevuto segnale: sigint\n", getpid());
    /* handler SIGINT */
    break;
    case SIGALRM:
      sm->flag = 1;

      for(i=0;i<(so_sources+so_taxi);i++){ /**/
   
        kill(vChild[i],SIGKILL);
      }
      sm->testStop = 1;
      

      
      
    /* handler SIGALRM */ 
    break;
    case SIGUSR1:
     
      printf("USER DEFINED RICEVUTO\n");
    break;
     
  }


}

void handler_signal_cmd(int signum){
 
  int n = 0, m = 0;
  switch (signum) {
    case SIGINT:
      printf("(%d) ricevuto segnale: sigint\n", getpid());
    /* handler SIGINT */
    break;
    case SIGALRM: 
    break;
    case SIGUSR1:
     
      srand(time(NULL) ^ getpid());
      do{
                 
          n = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
          m = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
                         
      }while((sm->mappa[n][m] == -1)||(n == k && m == l));
          msg.mtype = 2;
          sprintf(msg.dest,"%dc%dc%dc%dc",n,m,k,l);
          msgsnd(msg_id, &msg, sizeof(msg), 0);

      printf("(%d)SEGNALE USER DEFINED RICEVUTO  \n",getpid());
     
    break;
     
  }



}

void handler_stampa(int signal){
 struct timespec ns = {1,0};

  switch(signal){
    case SIGUSR2:
    if(sm->flag == 0){
      kill(getpid(), 0);
        if(errno!=ESRCH){
          /* system("clear");*/
            STAMPA_MAPPA_CONDIVISA();
            printf("\n");
            printf("\n");
            printf("\n");
            nanosleep(&ns, NULL);
            kill(getpid(),SIGUSR2);
        }
     
    }
    
    break;
  }
}