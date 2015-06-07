/*
 Name : exchanger.c
 Author : Rémi Dufour
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "defs.h"
//pthread_mutex_t ThreadNum_Mutex = PTHREAD_MUTEX_INITIALIZER;		

void* run(void* input){
	int exchangerNum = 0;

	//pthread_mutex_lock(&ThreadNum_Mutex);

	memcpy(&exchangerNum, &input, sizeof(int));
	printf("EXCHANGER thread %d START\n", exchangerNum);

	//pthread_mutex_unlock(&ThreadNum_Mutex);

	return NULL;
}			

int main(int argc, char* argv[]){

	printf("EXCHANGER process  START\n");

	int i = 0;
	pthread_t threadId[4];
	int ppid = getppid();
	

	for (i = 0 ; i<4 ; i++){
		pthread_create(&threadId[i], NULL, &run, (void*)i);
	}
	// signals the parent that all exchangers threads have been created
	kill(ppid, SIGUSR1);

	for (i = 0 ; i<4 ; i++){
		pthread_join(threadId[i], NULL);
	}
	
	printf("EXCHANGER process  END\n");

	return 0;
}