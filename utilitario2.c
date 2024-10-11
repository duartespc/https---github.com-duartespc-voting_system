#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "global.h"

/* Global Vars */
int source[3];
char fifo[6];

void *thread_function(void *threadid) {
	
 /* Declaring variables */
 char buffer[N_STR];
 int fd, n;
 
 /* Getting the values from Arguments Struct to use after */
 int t = (int) threadid;
 
 snprintf(fifo, sizeof(fifo), "fifo%d", t); // Choosing right fifo to open, according to args->thread_id
  
 char fileName[15];
 switch (t) {
	case 0:
		strcpy(fileName,"votantes_0.txt");
		break;
	case 1:
		strcpy(fileName,"votantes_1.txt");
		break;
	case 2:
		strcpy(fileName,"votantes_2.txt");
		break;
	default:
		break;
 }
 
 fd = open(fifo, O_WRONLY); // Opening the fifo - read only
 if ((source[t] = open(fileName, O_RDONLY)) == -1) {
	 printf("Error: file %s\n",fileName);
	 return 1;
 }
 while((n = read(source[t], buffer, N_STR)) > 0) { // While there are still lines to read from FIFO 
	write(fd, buffer, N_STR);  // Escrever no FIFO correspondente
 }
 close(fd);
 pthread_exit(NULL);
 return NULL;
}

void *write_stdin(void *threadid) {
	int fd, n;
	char buffer[N_STR];
	char fifa[6];
    int t = 3;
	 
	snprintf(fifa, sizeof(fifo), "fifo%d", t);
	fd = open(fifa, O_WRONLY); // Opening the fifo - write only

	while(1) {
		fgets(buffer, N_STR, stdin);
		write(fd, buffer, N_STR);  // Escrever no FIFO correspondente
	}
	
	close(fd);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[]) {
 pthread_t thread[N_THREADS+1];
 int i;

 /* Open Votantes0 file for reading */
 if ((source[0] = open("votantes_0.txt", O_RDONLY)) == -1) {
	printf("Error: file votantes_0\n");
	return 1;
 }
 /* Open Votantes1 file for reading */
 if ((source[1] = open("votantes_1.txt", O_RDONLY)) == -1) {
	printf("Error: file votantes_1\n");
	return 1;
 }
 /* Open Votantes2 file for reading */
 if ((source[2] = open("votantes_2.txt", O_RDONLY)) == -1) {
	printf("Error: file votantes_2\n");
	return 1;
 }

 
 /* Create fifos and threads */
 for (i=0; i < N_THREADS; i++) {
	
	printf("Create thread %d\n", i);
	if (pthread_create(&thread[i], NULL, &thread_function, (void *)i) != 0) {
		printf("pthread_create ERROR! thread: %d\n", i);
		return -1;
    }
 }

 if (pthread_create(&thread[3], NULL, &write_stdin, (void *)i) != 0) {
	printf("pthread_create ERROR! thread: 3\n");
	return -1;
 }
 
 /* Function to read from .txt file indicated in ARGV. NEEDS TO WRITE TO  FIFO */
 //read_write(source);


 /* Making sure threads were destroyed */
 pthread_exit(NULL);
 return 0;

}
