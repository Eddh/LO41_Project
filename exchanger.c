/*
 Name : exchanger.c
 Author : Rémi Dufour
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "defs.h"
//pthread_mutex_t ThreadNum_Mutex = PTHREAD_MUTEX_INITIALIZER;
Exchanger* shExchangers;
sem_t* semShExchangers;
sem_t* semShContinue;
int* p_continue;

int msgqId;


char *actionNames[4];

void* run(void* input){
	int exchangerIndex = 0, boolContinue;

	//pthread_mutex_lock(&ThreadNum_Mutex);

	memcpy(&exchangerIndex, &input, sizeof(int));
	printf("EXCHANGER thread %d START\n", exchangerIndex);

	CarMsg carMsg;
	Exchanger* thisExchanger = &shExchangers[exchangerIndex];
	printf("EXCHANGER index %d name %s\n", exchangerIndex, thisExchanger->name);
	do{
		msgrcv(thisExchanger->msgqId, &carMsg, sizeof(CarMsg) - sizeof(long), -3, 0);
		switch(carMsg.mtype){
			case 1 : // CarMsg
				printf("Exchanger %s has received a message from car %d which is %s\n", thisExchanger->name, carMsg.carId, actionNames[carMsg.action]);
			break;
			case 2 : // AuthAsk
			break;
			case 3 : // AuthAns
			break;
			default :
				printf("error exchanger.c:switch\n");
			break;
		}
	sem_wait(semShContinue);
	boolContinue = *p_continue;
	sem_post(semShContinue);
	}while(boolContinue);

	//pthread_mutex_unlock(&ThreadNum_Mutex);

	return NULL;
}			

int main(int argc, char* argv[]){

	printf("EXCHANGER process  START\n");

	int i = 0;
	pthread_t threadId[4];
	int ppid = getppid();
	key_t key, msgKey, msgKeyExchangers[4], contMsgqKey;
	int shmId = -1, shmContId = -1;
	int msgqIdExchangers[4];
	actionNames[0] = "ENTERING";
	actionNames[1] = "WAITING";
	actionNames[2] = "CROSSING";
	actionNames[3] = "LEAVING";

	//// ACCESSING SHARED MEMORY FOR EXCHANGERS
	key = ftok("main", 'a');
	if(key == -1){
		printf(" error exchanger.c:ftok errno : %d\n", errno);
	}
	shmId = shmget(key, sizeof(Exchanger*)*4, 0666);
	if(shmId == -1){
		printf("error exchanger.c:shmget errno : %d\n", errno);
	}
	shExchangers = (Exchanger*)shmat(shmId, (void*)0, 0);
	if(shExchangers == (Exchanger*)-1){
		printf("error exchangers:shmat errno %d %d\n", errno, shmId);
	}
	//// ACCESSING SHARED MEMORY FOR BOOLEAN "CONTINUE"
	contMsgqKey = ftok("main", 'b');
	if(contMsgqKey == -1){
		printf(" error ftok\n");
	}
	shmContId = shmget(contMsgqKey, sizeof(int), IPC_CREAT | 0666);
	if(shmContId == -1){
		printf("error shmget\n");
		shmctl(shmId, IPC_RMID, NULL);
	}
	p_continue = (int*)shmat(shmContId, (void*)0, 0);
	if(p_continue == (int*)-1){
		printf("error shmat errno %d %d %s\n", errno, shmId, argv[0]);
	}

	//// ACCESSING MAIN MESSAGE QUEUE
	msgKey = ftok("main", 'a');
	if(msgKey == -1){
		printf("error exchangers.c:ftok errno %d\n", errno);
	}
	msgqId = msgget(msgKey, 0666 | IPC_CREAT);
	if(msgqId == -1){
		printf("error exchangers.c:msgget errno%d\n", errno);
	}
	//// ACCESSING THE SEMAPHORE ON BOOLEAN CONTINUE
	semShContinue = sem_open("semShContinue", O_CREAT, 0666, 1);
	if(semShContinue == SEM_FAILED){
		printf("error exchangers.c:sem_open errno = %d, unspecified behavior\n", errno);
		sem_wait(semShContinue);
	}
	//// ATTRIBUTING MESSAGES QUEUES TO EACH EXCHANGER THREAD
	semShExchangers = sem_open("shExchangers", 0);
	if(semShExchangers == SEM_FAILED){
		printf("error exchangers.c:sem_open errno = %d, unspecified behavior\n", errno);
		sem_wait(semShExchangers);
	}
	

	for (i = 0 ; i < 4 ; i++){
		msgKeyExchangers[i] = ftok("main", i+1);
		if(msgKeyExchangers[i] == -1){
			printf("error cars.c:ftok errno %d\n", errno);
		}
		msgqIdExchangers[i] = msgget(msgKeyExchangers[i], 0666 | IPC_CREAT);
		if(msgqIdExchangers[i] == -1){
			printf("error cars.c:msgget errno%d\n", errno);
		}
		shExchangers[i].msgqId = msgqIdExchangers[i];
	}

	// thread creation
	for (i = 0 ; i<4 ; i++){
		pthread_create(&threadId[i], NULL, &run, (void*)i);
	}
	sem_post(semShExchangers);
	// signals the parent that all exchangers threads have been created
	kill(ppid, SIGUSR1);

	for (i = 0 ; i<4 ; i++){
		pthread_join(threadId[i], NULL);
	}
	
	printf("EXCHANGER process  END\n");

	return 0;
}