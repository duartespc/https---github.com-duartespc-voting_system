#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "global.h"
#define N_THREADS_OUT 10

int votos[13];

char fifo[6];
pthread_mutex_t mut; // declare mutex
pthread_mutex_t mutResultados; // declare mutex
time_t timet;

int bignum = 0;
int fdEventos, fdResultados;

void write_to_eventos(char *identificador, long int tempo_entrada, long int tempo_saida) {
	char str[90];
	time_t marca_tempo_entrada = (time_t)tempo_entrada;
	time_t marca_tempo_saida = (time_t)tempo_saida;

	char entrada[26];
	strncpy(entrada, ctime(&marca_tempo_entrada), 26);
	char saida[26];
	strncpy(saida, ctime(&marca_tempo_saida), 26);
	
	if (entrada[strlen(entrada)-1] == '\n') 
		entrada[strlen(entrada)-1] = '\0';
	if (saida[strlen(saida)-1] == '\n') 
		saida[strlen(saida)-1] = '\0';
		
		
	// try sprintf(entrada, marca-tempo_entrada)
	char *entradaPtr = entrada;
	char *saidaPtr = saida;

    sprintf(str,"%s, %s, %s\n", identificador, entradaPtr, saidaPtr);

    write(fdEventos, str, 90);
}

int get_int_len (int value){
  int l=1;
  while(value>9){ l++; value/=10; }
  return l;
}

void update_resultados() {
	if ((fdResultados = open("resultados.txt", O_WRONLY|O_CREAT|O_TRUNC, 0777)) == -1) {
		printf("Error: file resultados.txt");
		return 1;
    }  
	int i;
	for(i = 0; i < 13; i++)
	{
		char str[18]; 
		sprintf(str,"Choice %d : %d\n", i+1, votos[i]);
		write(fdResultados, str, 11+get_int_len(i+1)+get_int_len(votos[i]));
	}
	close(fdResultados);
}

void *read_from_fifo(void *threadid) {
    int t = (int)threadid; 
    int fd, n;
   
    snprintf(fifo, sizeof(fifo), "fifa%d", t);
    fd = open(fifo, O_RDONLY);
    char buffer[100]; 
	
    while ((n = read(fd, buffer, 100)) > 0){
	    int i = 0;
	    const char s[2] = ",";
		char *token = strtok (buffer, s);
		char *array[5];
	
		while (token != NULL) {
			array[i++] = token;
			token = strtok (NULL, s);
		}
		
		char identificador[36];
		long int tempo_entrada;
		long int tempo_saida;
		int item_voto;
		
		strncpy(identificador, array[0], 36);
		item_voto = atoi(array[3]);
		tempo_entrada = atoi(array[4]);
		tempo_saida = atoi(array[5]);
		
		pthread_mutex_lock(&mut); // mutex lock  
		   
		votos[item_voto-1]++;
	    write_to_eventos(identificador, tempo_entrada, tempo_saida);   // critical section
		
		pthread_mutex_unlock(&mut); // mutex unlock

		pthread_mutex_lock(&mutResultados); // mutex lock  

		update_resultados();
		
		pthread_mutex_unlock(&mutResultados); // mutex unlock

		//printf("%s\n", buffer);
		printf("ID: %s TempoEntrada: %ld TempoSaida: %ld\n", identificador, tempo_entrada, tempo_saida);
    }	

    close(fd); 
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
  int i;
  pthread_t thread[N_THREADS_OUT];
  
  pthread_mutex_init(&mut, NULL); // initialize mutex
  pthread_mutex_init(&mutResultados, NULL); // initialize mutex
  srand((unsigned) time(&timet));


  
  if ((fdEventos = open("eventos.txt", O_WRONLY|O_CREAT|O_TRUNC, 0777)) == -1){
	   printf("Error: file eventos.txt");
	   return 1;
  }
  
  // char str[11];  

  // pthread_mutex_lock(&mut); // mutex lock 
  // for(i = 0; i < 14; i++) {
	  // sprintf(str,"Item %d - \n", i+1);
	  // if (i < 9)
		// write(fdResultados, str, 10);
	  // else
		// write(fdResultados, str, 11);
  // }
  // pthread_mutex_unlock(&mut); // mutex unlock


  for(i=0; i < N_THREADS_OUT; i++) {
     printf("Create thread %d\n", i);
     pthread_create(&thread[i], NULL, read_from_fifo, (void *)i);
  }
  
  for(i=0; i < N_THREADS_OUT; i++) {
	  pthread_join(thread[i], NULL);
  }
  
  close(fdEventos);
  pthread_mutex_destroy(&mut); // destroy mutex
  pthread_exit(NULL);
  return 0;	
}