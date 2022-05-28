#define READ_BUF_SIZE 12
#define MY_OUT_TAXI "time.txt"
#define MY_OUT_SOURCES "gen_sources.txt"
#define MY_OUT_DEST_MASTER "gen_dest_master.txt"
#define MY_OUT_CLIENTI "num_clienti.txt"
#define SO_HEIGHT 15
#define SO_WIDTH 15
#define NUM_ITER 3
#define MSG_LEN 120
#define MY_KEY 0x123456
#define ROSSO_TAXI          \
  sem.sem_num = 1;     \
  sem.sem_op = -1;     \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define VERDE_TAXI          \
  sem.sem_num = 1;     \
  sem.sem_op = 1;      \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define ROSSO_MASTER         \
  sem.sem_num = 4;     \
  sem.sem_op = -1;     \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define VERDE_MASTER          \
  sem.sem_num = 4;     \
  sem.sem_op = 1;      \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);  
#define ROSSO_DEST        \
  sem.sem_num = 3;     \
  sem.sem_op = -1;     \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define VERDE_DEST          \
  sem.sem_num = 3;     \
  sem.sem_op = 1;      \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define ROSSO_RICHIESTE          \
  sem.sem_num = 2;     \
  sem.sem_op = -1;     \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);
#define VERDE_RICHIESTE          \
  sem.sem_num = 2;     \
  sem.sem_op = 1;      \
  sem.sem_flg = 0;     \
  semop(sem_id, &sem, 1);  
   

#define STAMPA_MAPPA_CONDIVISA() {  for(k=0;k<SO_WIDTH;k++){ \
                           for(l=0;l<SO_HEIGHT;l++){ \
                              if(sm->mappa[k][l] == -1)\
                                 printf("|_%d _|",sm->mappa[k][l]); \
                              else \
                                printf("|_ %d _|",sm->mappa[k][l]); \
                           }\
                           printf("\n"); \
                        } \
                     } 

#define STAMPA_RICHIESTE_CONDIVISA() {  for(k=0;k<SO_WIDTH;k++){ \
                           for(l=0;l<SO_HEIGHT;l++){ \
                              if(sm->richieste[k][l] == -1)\
                                 printf("|_%d _|",sm->richieste[k][l]); \
                              else \
                                printf("|_ %d _|",sm->richieste[k][l]); \
                           }\
                           printf("\n"); \
                        } \
                     }   

#define STAMPA_DEST_CONDIVISA() {  for(k=0;k<SO_WIDTH;k++){ \
                           for(l=0;l<SO_HEIGHT;l++){ \
                                 printf("(%d,%d) %d",k,l,sm->destination[k][l]); \
                           }\
                           printf("\n"); \
                        } \
                     }  
#define RESET_DEST() {  for(n=0;n<SO_WIDTH;n++){ \
                           for(m=0;m<SO_HEIGHT;m++){ \
                                if(sm->destination[n][m] != -1) \
                                 sm->destination[n][m] = 0; \
                           }\
                        } \
                     }        

struct shared_memory {
  int inactive;
  int richieste[SO_WIDTH][SO_HEIGHT]; 
  int mappa[SO_WIDTH][SO_HEIGHT];
  int destination[SO_WIDTH][SO_HEIGHT];
  int top_cells[SO_WIDTH*SO_HEIGHT];
  int successi;
  int id;
  int viaggi;
  int vDestX[100000];
  int vDestY[100000];
  int count_abort;
  int vSourcesX[100000];
  int vSourcesY[100000];
  int flag;
  int tcount;
  int scount;
  int testStop;
  int index;
  int index2;
};

struct msgbuff {
  long mtype;             /* message type, must be > 0 */
  char dest[MSG_LEN];    /* message data */
};


void generazione_holes(int so_holes);
void inizializzazione_mappa_richieste();