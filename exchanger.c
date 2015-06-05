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

pthread_mutex_t ThreadNum_Mutex = PTHREAD_MUTEX_INITIALIZER;		

void* run(void* in){
	int thisThreadNum = 0;

	pthread_mutex_lock(&ThreadNum_Mutex);

	memcpy(&thisThreadNum, &in, sizeof(int));
	printf("EXCHANGER thread number %d START\n", thisThreadNum);

	pthread_mutex_unlock(&ThreadNum_Mutex);

	return NULL;
}			

int main(int argc, char* argv[]){

	printf("EXCHANGER process\tSTART\n");

	int i = 0;
	pthread_t threadId[4];

	for (i = 0 ; i<4 ; i++){
		pthread_create(&threadId[i], NULL, &run, (void*)i);
	}
	for (i = 0 ; i<4 ; i++){
		pthread_join(threadId[i], NULL);
	}
	
	printf("EXCHANGER process\tEND\n");

	return 0;
}