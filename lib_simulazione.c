#include "lib_simulazione.h"
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
                      


void generazione_holes(int so_holes){

int z = 0, k = 0, l = 0, sm_id = 0;
struct shared_memory * sm;

FILE * fsm = fopen("var.txt","r");
fscanf(fsm,"%d",&sm_id);
fclose(fsm);

sm = shmat(sm_id,NULL,0);

srand(time(NULL) ^ getpid());
z = (SO_WIDTH*SO_HEIGHT)/9;


if(so_holes <= z){
   while(so_holes > 0){
      k = rand() % ((SO_WIDTH-1) + 1 - 0) + 0;
      l = rand() % ((SO_HEIGHT-1) + 1 - 0) + 0;
       if(sm->mappa[k+1][l]!= -1 && sm->mappa[k-1][l]!= -1 && sm->mappa[k][l+1]!= -1 && sm->mappa[k][l-1]!= -1 && sm->mappa[k-1][l-1]!= -1 && sm->mappa[k-1][l+1]!= -1 && sm->mappa[k+1][l-1]!= -1 && sm->mappa[k+1][l+1]!= -1){
          sm->mappa[k][l] = -1;
          sm->destination[k][l] = -1;
          sm->richieste[k][l] = -1;
       
          so_holes--;
        }
    }
 }else{
    perror("Troppi holes per la dimensione della mappa");
    exit(0);
   }


}


void inizializzazione_mappa_richieste(){

	int i = 0, j = 0, sm_id = 0;
	struct shared_memory * sm;

	FILE * fsm = fopen("var.txt","r");
	fscanf(fsm,"%d",&sm_id);
	fclose(fsm);

	sm = shmat(sm_id,NULL,0);


	for(i=0;i<SO_WIDTH;i++){ 
	  for(j = 0;j<SO_HEIGHT;j++){
	      sm->richieste[i][j] = 0;
	  }
	    
	}
	    


	 for(i=0;i<SO_WIDTH;i++){ 
	  for(j = 0;j<SO_HEIGHT;j++){
	      sm->mappa[i][j] = 0;
	  }
	    
	}


}