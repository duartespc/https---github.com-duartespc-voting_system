#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "global.h"
#include <time.h>

// MAXRINGQUEUE must be 2^x (16,32,64,...)  2^17 = 131072
#define MAXRINGQUEUE (131072) 
#define MAXQUEUE (131072)

/* Global Vars */

// Ring QUEUE semaphore
sem_t *fullRQ;
int valueRQ;

// Assembly QUEUES (0-9) semaphores
sem_t *fullAssemblyQ[10];
int value[10];


pthread_mutex_t mutRQ; // declare mutex
// Assembly queues (0-9) mutex, to ensure pop() doesn't mess thing up...
pthread_mutex_t mutAssembly[10]; // declare mutex

typedef enum { FALSE, TRUE } boolean;

void *verify_and_stamp(void *voteToVerify);
void *ler_do_pipe(void *threadid);
void *stack_in_queues(void *i);
void *assembly_table(void *threadid);

struct vt queueRing[MAXRINGQUEUE]; /* Main Queue - Ring */

struct vt queue[10][MAXQUEUE];

time_t timet;

/* Main queue (ring queue) */
int ringQueueIn = 0; // beginning 
int ringQueueOut = 0 ; // end
void ringQueuePush(struct vt d);
struct vt ringQueuePop();
void ringQueueDisplay();
void ringQueueClean(int i);

/* Queue 0-9 for FIFO-OUT */
int in[10]; // beginning
int out[10]; // end
boolean PushToAssembly(int queueId, struct vt d);
struct vt pop(int queueId);
void display(int queueId);
void clean(int queueId, int index);
void reset(int queueId);
boolean isEmpty(int queueId);


char fifo[6];
char fifa[6];

pthread_t thread[N_THREADS+1];
pthread_t threadout[N_THREADS_OUT];
pthread_t threadout2[N_THREADS_OUT];


/*Reading buffer (one vote at a time) from FIFO, pushing it to MAIN Queue and sending it to the verification thread (verify_and_stamp) */
void *ler_do_pipe(void *threadid) {

 int t = (int)threadid;
 char buffer[N_STR];
 int n, fd;
 snprintf(fifo, sizeof(fifo), "fifo%d", t);
 fd = open(fifo, O_RDONLY);

 while ((n = read(fd, buffer, N_STR)) > 0) { 

	struct vt vote;
	int i = 0;
	const char s[3] = ", ";
	char *token = strtok (buffer, s);
	char *array[4];

	while (token != NULL) {
		array[i++] = token;
		token = strtok (NULL, s);
	}

	if(strlen(array[0]) == 36 && strlen(array[1]) == 6 && strlen(array[2]) == 2) {
	
		char mesabuffer[3];
		char identificador[36];

		strncpy(mesabuffer, array[2], 1);
		int assembleia = atoi(array[1]);
		char mesa_voto = mesabuffer[0];

		strncpy(vote.identificador, array[0], 36);
		vote.assembleia = assembleia;
		vote.mesa_voto = mesa_voto;
	
		
		sem_getvalue(fullRQ, &valueRQ);
        sem_wait(fullRQ);
        sem_getvalue(fullRQ, &valueRQ);
		ringQueuePush(vote);
		//printf("%s %d %c\n", vote.identificador,vote.assembleia,vote.mesa_voto);
	}
 }
 close(fd);
 pthread_exit(NULL);
}

void *ler_do_stdin(void *threadid) {
	int fd, n;
	int t = 3;
	char str[6];
	char buffer[N_STR];
	snprintf(str, sizeof(fifo), "fifo%d", t);
	
	fd = open(str, O_RDONLY);
	
	while ((n = read(fd, buffer, N_STR)) > 0) { 
		struct vt vote;
		int i = 0;
		const char s[3] = ", ";
		char *token = strtok (buffer, s);
		char *array[4];

		while (token != NULL) {
			array[i++] = token;
			token = strtok (NULL, s);
		}

		if(strlen(array[0]) == 36 && strlen(array[1]) == 6 && strlen(array[2]) == 2) {
	
			char mesabuffer[3];
			char identificador[36];

			strncpy(mesabuffer, array[2], 1);
			int assembleia = atoi(array[1]);
			char mesa_voto = mesabuffer[0];

			strncpy(vote.identificador, array[0], 36);
			vote.assembleia = assembleia;
			vote.mesa_voto = mesa_voto;
	
		
			sem_getvalue(fullRQ, &valueRQ);
			sem_wait(fullRQ);
			sem_getvalue(fullRQ, &valueRQ);
			ringQueuePush(vote);
			printf("%s %d %c\n", vote.identificador,vote.assembleia,vote.mesa_voto);
		}
	}
	close(fd);
	pthread_exit(NULL);
}

void *verify_and_stamp(void *i) {

struct vt *vote = i;

 if (  (strlen(vote->identificador) == 36) && (vote->assembleia == 110632) && (vote->mesa_voto >= 'A' && vote->mesa_voto <= 'J') ) {
	vote->marca_tempo_entrada = (long int)time(NULL);
	sleep(rand() % 3 + 1); // Wait 1-3 seconds (random)
 }
 
 pthread_exit(NULL);
 return NULL;
}

void *stack_in_queues(void *i) {
	
	while (1) {
		 
		if (ringQueueIn != ringQueueOut) {
			
			pthread_mutex_lock(&mutRQ); // mutex lock  
			struct vt vote = ringQueuePop();
			pthread_mutex_unlock(&mutRQ); // mutex unlock  

			int tid = -1;
	  	  
			switch(vote.mesa_voto) 
			{
				case 'A':
					tid = 0;
					break;
				case 'B':
					tid = 1;
					break;
				case 'C':
					tid = 2;
					break;
				case 'D':
					tid = 3;
					break;
				case 'E':
					tid = 4;	
					break;
				case 'F':
					tid = 5;
					break;
				case 'G':
					tid = 6;
					break;
				case 'H':
					tid = 7;
					break;
				case 'I':
					tid = 8;
					break;
				case 'J':
					tid = 9;
					break;	
				default:
					break;
			}
	  
			if(tid != -1) 
			{
				//printf("%s entering assembly %d\n",  vote.identificador, tid);
				sem_getvalue(fullAssemblyQ[tid], &value[tid]);
				sem_wait(fullAssemblyQ[tid]);
				if (PushToAssembly(tid, vote) == FALSE)
					printf("Stack in queue[%d]->pushtoAssembly -> Queue overflow \n", tid);
				sem_getvalue(fullAssemblyQ[tid], &value[tid]);
			}
			
		}
	}
	pthread_exit(NULL);
	return NULL;
}

/* One thread for each Assembly Table, removes the votes from the respective Queue and sends it via FIFO */
void *assembly_table(void *threadid) {
	
	int t = (int) threadid;
	int fd, n;
	char buffer[100]; // N_STR + NEW CHARACTERS FOR VOTE AND TIME STAMP
	struct vt vote;
	snprintf(fifa, sizeof(fifo), "fifa%d", t);
    fd = open(fifa, O_WRONLY);
		
	while (1)
	{
	
		pthread_mutex_lock(&mutAssembly[t]); // mutex lock  
		vote = pop(t);
		sem_post(fullAssemblyQ[t]);
		pthread_mutex_unlock(&mutAssembly[t]); // mutex lock  

		
		if (  (strlen(vote.identificador) == 36) && (vote.assembleia == 110632) ) {
			
			vote.item_voto = rand() % 13 + 1;
			sleep(1);
			vote.marca_tempo_saida = (long int)time(NULL);
			
			char *tempo_saida = asctime(localtime(&vote.marca_tempo_saida));
			tempo_saida[strlen(tempo_saida) - 1] = 0;
			
			char *tempo_entrada = asctime(localtime(&vote.marca_tempo_entrada));
			tempo_entrada[strlen(tempo_entrada) - 1] = 0;
	
			sprintf(buffer, "%s,%d,%c,%d,%ld,%ld", vote.identificador, vote.assembleia, vote.mesa_voto, vote.item_voto, vote.marca_tempo_entrada, vote.marca_tempo_saida);
			printf("%s %d %c %d %ld %ld \n", vote.identificador, vote.assembleia, vote.mesa_voto, vote.item_voto, vote.marca_tempo_entrada, vote.marca_tempo_saida);
			
			write(fd, buffer, 100);  // Escrever no FIFO correspondente
	
		}
	}
	close(fd);
	pthread_exit(NULL);
	return NULL;
}


int main() {
	
 srand((unsigned) time(&timet));
 int i;
 
 
 // Inititalizing semaphores used for RingQueue and assembly Tables...
 fullRQ = sem_open("/sem1", O_CREAT, 0666, MAXRINGQUEUE-1); 
 
 fullAssemblyQ[0] = sem_open("/sem2", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[1] = sem_open("/sem3", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[2] = sem_open("/sem4", O_CREAT, 0666, MAXQUEUE-1);
 fullAssemblyQ[3] = sem_open("/sem5", O_CREAT, 0666, MAXQUEUE-1);
 fullAssemblyQ[4] = sem_open("/sem6", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[5] = sem_open("/sem7", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[6] = sem_open("/sem8", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[7] = sem_open("/sem9", O_CREAT, 0666, MAXQUEUE-1);
 fullAssemblyQ[8] = sem_open("/sem10", O_CREAT, 0666, MAXQUEUE-1); 
 fullAssemblyQ[9] = sem_open("/sem11", O_CREAT, 0666, MAXQUEUE-1);
 
 for(i=0; i<N_THREADS+1; i++) {
 snprintf(fifo, sizeof(fifa), "fifo%d", i);
 mkfifo(fifo, 0666);
 }
 
 for(i=0; i<N_THREADS_OUT; i++) {
 snprintf(fifa, sizeof(fifa), "fifa%d", i);
 mkfifo(fifa, 0666);
 }
 
 pthread_mutex_init(&mutRQ, NULL);

 /*Initializing the values for the Assembly queues and Mutexes*/
 for(i = 0; i < 10; i++) {
	in[i] = -1; // beginning
	out[i] = -1; // end
	pthread_mutex_init(&mutAssembly[i], NULL); // initialize mutex
 }
 
 printf("Launch ./utilitario3 to enable logs (resultados and eventos)!\nLaunch ./utilitario2 to read input\n");

 /* Creating threads to read from INPUT FIFOS (ler_do_pipe) and pushing them into the RING Queue
  When you ringQueuePush(vote) it creates a verification thread which puts it a timestamp */
 for(i=0; i< N_THREADS; i++) {
    printf("Create thread %d\n", i);
    pthread_create(&thread[i], NULL, ler_do_pipe, (void *)i);
 }
 
 pthread_create(&thread[3], NULL, ler_do_stdin, (void *)i);
 

 /* Creating threads to PULL votes from the RingQueue and pushing them to respective Assembly Table  */
 for (i=0; i < N_THREADS_OUT; i++) {
	 if (pthread_create(&threadout[i], NULL, stack_in_queues, (void *)i) != 0) {
		printf("pthread_create ERROR! thread: %d", i);
		return -1;
    }
 }
 
 /* Creating Threads and Fifos for each Assembly Table, to pull all votes from respective table Queue
   and create "resultandos.txt" and "eventos.txt" with information about voters/votes. */
 for(i=0; i < N_THREADS_OUT; i++) 
 {
	 pthread_create(&threadout2[i], NULL, assembly_table, (void *)i);
 }

 
 pthread_exit(NULL);
 return EXIT_SUCCESS;
 
}


/* insert item in Main Queue (Ring queue) */
void ringQueuePush(struct vt d){
	pthread_t th;

	pthread_create(&th, NULL, verify_and_stamp, (void *)&d); // Verification thread - put the timestamp

	int index;
	
	sleep(1);
	index = ((ringQueueIn++) & (MAXRINGQUEUE-1));
	queueRing[index] = d;
}

/* insert item in Assembly queue[id]*/
boolean PushToAssembly(int queueId, struct vt d) {
    if (out[queueId] == MAXQUEUE - 1){
        return FALSE ;
    } else {
        if (in[queueId] == - 1) {
            /* queue is empty */
            in[queueId] = 0;
        }    
        out[queueId] = out[queueId] + 1;
        queue[queueId][out[queueId]] = d;
    }	
    return TRUE;
} 

/* remove item */
struct vt ringQueuePop(){
	int index;
	index = ((ringQueueOut++) & (MAXRINGQUEUE-1));
	struct vt r;
  	r = queueRing[index];
  	ringQueueClean(index);
	if (r.assembleia == 110632)
		sem_post(fullRQ);
  	return r;
}

/* remove item in Assembly queue[id] */
struct vt pop(int queueId) {
	struct vt r;
    if (in[queueId] >= 0 || in[queueId] < out[queueId]) {
    	r = queue[queueId][in[queueId]];
        clean(queueId,in[queueId]);
        in[queueId] = in[queueId] + 1;

        if (isEmpty(queueId))
        	reset(queueId);
		
    }     
    return r;
} 

/* print queueRing */
void ringQueueDisplay() {
    int i;
    for (i=0; i < ringQueueIn; i++) {
		char identificadorteste[36];
		strcpy(identificadorteste,queueRing[i].identificador);
		printf("queue[%d] = %s, %d, %c\n", i, identificadorteste, queueRing[i].assembleia, queueRing[i].mesa_voto);
	}
}

/* print assembly queue[id] */
void display(int queueId) {
    int i;
    if (in[queueId] == - 1){
        printf("Empty queue\n");
    } else {
        for (i = in[queueId]; i <= out[queueId]; i++) {
			char identificadorteste[36];
			strcpy(identificadorteste,queue[queueId][i].identificador);
			printf("queue[%d][%d] = %s, %d, %c\n", queueId, i, identificadorteste, queue[queueId][i].assembleia, queue[queueId][i].mesa_voto);
		}
    }
} 

/* ringQueueClean item */
void ringQueueClean(int i) {
    queueRing[i].identificador[0] = 0;
    queueRing[i].assembleia = 0;  
    queueRing[i].mesa_voto = 0;  
    queueRing[i].item_voto = 0;
    queueRing[i].marca_tempo_entrada = 0;
    queueRing[i].marca_tempo_saida = 0;	
}

/* clean item in assembly queue[id] */ 
void clean(int queueId, int index) {
	queue[queueId][index].identificador[0] = 0;
    queue[queueId][index].assembleia = 0;  
	queue[queueId][index].mesa_voto = 0;  
	queue[queueId][index].item_voto = 0;
	queue[queueId][index].marca_tempo_entrada = 0;
	queue[queueId][index].marca_tempo_saida = 0;
}

/* reset assembly queue[id] */ 
void reset(int queueId) {
	in[queueId] = -1;
   	out[queueId] = -1;
   	memset(&queue[queueId], 0, sizeof(queue[queueId]));
}

boolean isEmpty(int queueId) {
	if (in[queueId] == -1 || in[queueId] > out[queueId]) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}
