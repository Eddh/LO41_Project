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
int msgqIdExchangers[4];


char *actionNames[4];

void sigIntHandler(int sig){
	EndMsg endMsg;
	endMsg.mtype = 4;
	int i = 0;

	sem_wait(semShContinue);
	*p_continue = 0;
	sem_post(semShContinue);
	for(i = 0 ; i < 4 ; i++){
		msgsnd(msgqIdExchangers[i], &endMsg, sizeof(EndMsg) - sizeof(long), 0);
	}
	printf("hello\n");
}

void* run(void* input){
	int exchangerIndex = 0, boolContinue;

	

	memcpy(&exchangerIndex, &input, sizeof(int));
	printf("EXCHANGER thread %d START\n", exchangerIndex);

	CarMsg carMsg;
	Exchanger* thisExchanger = &shExchangers[exchangerIndex];
	printf("EXCHANGER index %d name %s\n", exchangerIndex, thisExchanger->name);
	do{
		msgrcv(thisExchanger->msgqId, &carMsg, sizeof(CarMsg) - sizeof(long), -4, 0);
		switch(carMsg.mtype){
			case 1 : // CarMsg
				//printf("Exchanger %s has received a message from car %d which is %s\n", thisExchanger->name, carMsg.carId, actionNames[carMsg.action]);
			break;
			case 2 : // AuthAsk
				printf("Exchanger %s has received a message from car %d which asks to cross\n", thisExchanger->name, carMsg.carId);
			break;
			case 3 : // AuthAns
			break;
			case 4 : // EndMsg

			default :
				printf("error exchanger.c:switch\n");
			break;
		}
		sem_wait(semShContinue);
		boolContinue = *p_continue;
		sem_post(semShContinue);
	}while(boolContinue);
	printf("THREAD EXCHANGER %d END\n", exchangerIndex);


	return NULL;
}			

int main(int argc, char* argv[]){

	printf("EXCHANGER process  START\n");

	int i = 0;
	pthread_t threadId[4];
	int ppid = getppid();
	key_t key, msgKey, msgKeyExchangers[4], contShmKey;
	int shmId = -1, shmContId = -1;
	
	actionNames[0] = "ENTERING";
	actionNames[1] = "WAITING";
	actionNames[2] = "CROSSING";
	actionNames[3] = "LEAVING";

	//// PREPARATION FOR SIGINT HANDLER
	struct sigaction sa_SigInt;
	sa_SigInt.sa_handler = sigIntHandler;
	sa_SigInt.sa_flags = 0;
	sigemptyset(&sa_SigInt.sa_mask);

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
	contShmKey = ftok("main", 'b');
	if(contShmKey == -1){
		printf(" error ftok\n");
	}
	shmContId = shmget(contShmKey, sizeof(int), IPC_CREAT | 0666);
	if(shmContId == -1){
		printf("error shmget\n");
		shmctl(shmContId, IPC_RMID, NULL);
	}
	p_continue = (int*)shmat(shmContId, (void*)0, 0);
	if(p_continue == (int*)-1){
		printf("error shmat errno %d %d %s\n", errno, shmId, argv[0]);
	}
	printf(" exchanger shmContId : %d\n",shmContId);

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
	semShContinue = sem_open("semShContinue", 0);
	if(semShContinue == SEM_FAILED){
		printf("error exchangers.c:sem_open errno = %d, unspecified behavior\n", errno);
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
	sem_post(semShExchangers);
	// thread creation
	for (i = 0 ; i<4 ; i++){
		pthread_create(&threadId[i], NULL, &run, (void*)i);
	}
	
	// signals the parent that all exchangers threads have been created
	if(sigaction(SIGINT, &sa_SigInt, NULL) == -1){
		printf("error main.c : main : sigaction\n");
	}
	kill(ppid, SIGUSR1);

	for (i = 0 ; i<4 ; i++){
		pthread_join(threadId[i], NULL);
		printf("okkk thread %d joined\n\n\n", i);
	}
	
	//// REMOVING MESSAGE QUEUES FOR EACH EXCHANGER
	for (i = 0 ; i < 4 ; i++){
		msgctl(msgqIdExchangers[i], IPC_RMID, NULL);
	}

	printf("EXCHANGER process  END\n");

	return 0;
}