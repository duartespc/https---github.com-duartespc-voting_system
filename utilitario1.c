/******************************************************************************
 * FILE: IO_write_uuid_file.c
 * DESCRIPTION:
 * Write n lines (uuid) to file
 * Usage: ./IO_write_uuid_file <file_name> <n_lines>
 * 
 * Example:
 * ./IO_write_uuid_file file.txt 100
 * 
 * AUTHOR: Jose G. Faisca
 * LAST REVISED: 2020/11
 ******************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <semaphore.h>
#include <signal.h>
#include "global.h"

void write_to_file(int fd);
char *getuuid();

int main(int argc, char *argv[]){
    int fd, n_lines, i;
    time_t t;
    
    srand((unsigned) time(&t));
	
    if (argc != 3){
	printf("Usage: %s <file_name> <n_lines>\n",argv[0]);
	return 1;
    }
	
    if ((fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0664)) == -1){
	printf("Error: file %s",argv[2]);
	return 1;
    }
	
    n_lines = atoi(argv[2]);
     
    for (i=0; i<n_lines; i++){
	   write_to_file(fd);
	}
	
    close(fd);
    
    return 0;
}

void write_to_file(int fd) {
	int assembleia = 110632;
    char str[N_STR];
    sprintf(str,"%s, %d, %s\n", getuuid(), assembleia, mv[rand() % N_MV]);
    write(fd, str, N_STR);
}

char *getuuid(){    
	static char str[36]; 
	uuid_t uuid;
	uuid_clear(uuid);
	uuid_generate_random(uuid);
	uuid_unparse(uuid,str); 
	return str; 
}


