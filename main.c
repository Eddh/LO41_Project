/*
 Name : main.c
 Author : Rémi Dufour
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "defs.h"


int* p_continue;
sem_t* semShContinue;

void* sigusr1_handler(int sig)
{
	return 0;
}
void* sigIntHandler(int sig){

	printf("STOP\n");
	sem_wait(semShContinue);
	*p_continue = 0;
	sem_post(semShContinue);
	return 0;
}

// void* inputEventThread(){

// 	return NULL;
// }

int main(int argc, char *argv[]){
	printf("PROGRAM START\n");
/*******************************************************************				
				  DECLARATIONS & INITIALIZATIONS
********************************************************************/
	int pid, pid_exchanger, pid_server, pid_cars;
	int nbCars = NBCARS;
	int shmId = 0, shmContId = 0, msgqId;
	char nbCarsString[32];
	key_t key, msgKey, contMsgqKey;
	char* input = NULL;
	size_t len = 0, nbCarsStringLen = 32;
	ssize_t read = 0;
	Exchanger* shExchangers = NULL;
	sem_t* semShExchangers;

	struct sigaction sa;
	sa.sa_handler = sigusr1_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGUSR1, &sa, NULL) == -1){
		printf("error main.c : main : sigaction\n");
	}

	sigset_t letSigUsr1Mask;
	sigfillset(&letSigUsr1Mask);
	sigdelset(&letSigUsr1Mask, SIGUSR1);

	struct sigaction sa_SigStop;
	sa_SigStop.sa_handler = sigIntHandler;
	sa_SigStop.sa_flags = 0;
	sigemptyset(&sa_SigStop.sa_mask);


	//// CREATING THE SHARED MEMORY SEGMENT WHICH WILL CONTAIN EXCHANGERS
	key = ftok("main", 'a');
	if(key == -1){
		printf(" error ftok\n");
	}
	shmId = shmget(key, sizeof(Exchanger)*4, IPC_CREAT | 0666);
	if(shmId == -1){
		printf("error shmget\n");
		shmctl(shmId, IPC_RMID, NULL);
	}
	shExchangers = (Exchanger*)shmat(shmId, (void*)0, 0);
	if(shExchangers == (Exchanger*)-1){
		printf("error shmat errno %d %d %s\n", errno, shmId, argv[0]);
	}

	//// CREATING SHARED MEMORY SEGMENT FOR BOOLEAN "CONTINUE"
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
	*p_continue = 1;
	//// CREATING THE MESSAGE QUEUE
	msgKey = ftok("main", 'a');
	if(msgKey == -1){
		printf("error cars.c:ftok errno %d\n", errno);
	}
	msgqId = msgget(msgKey, 0666 | IPC_CREAT);
	if(msgqId == -1){
		printf("error cars.c:msgget errno%d\n", errno);
	}

	//// CREATING THE SEMAPHORE ON SHEXCHANGERS
	semShExchangers = sem_open("shExchangers", O_CREAT, 0666, 1);
	if(semShExchangers == SEM_FAILED){
		printf("error exchangers.c:sem_open errno = %d, unspecified behavior\n", errno);
		sem_wait(semShExchangers);
	}
	//// CREATING THE SEMAPHORE ON BOOLEAN CONTINUE
	semShContinue = sem_open("semShContinue", O_CREAT, 0666, 1);
	if(semShContinue == SEM_FAILED){
		printf("error exchangers.c:sem_open errno = %d, unspecified behavior\n", errno);
		sem_wait(semShContinue);
	}

	create4ExchangersCity(shExchangers);
	//// USER INPUT
	do {
		printf("please enter the number of cars you wish(1 to 1000), nothing->%d : ", NBCARS);
		printf("\b\b\b\b\b\b\b\b\b                ");
		if(read = getline(&input, &len, stdin) == -1){
		printf("error main.c:main:getline\n");
		}
		nbCars = atoi(input);
	} while(nbCars < 0 || nbCars > 1000);
	if(nbCars == 0){ // default number of cars
		nbCars = NBCARS;
	}
	snprintf(nbCarsString, nbCarsStringLen, "%d", nbCars);


/*******************************************************************				
				  PROCESS CREATION : SERVER EXCHANGER CARS
********************************************************************/
	switch(pid_server = fork()){
		case -1 : // error
			printf("error fork server\n");
		break;
		case 0 : // child
			if(execl("./server", "server", NULL) == -1){
				printf("error execl server\n");
			}
		break;
		default : // parent
			switch(pid_exchanger = fork()){
				case -1 : // error

					printf("error fork exchanger\n");
				break;
				case 0 : // child
					if(execl("./exchanger", "exchanger", NULL) == -1){
						printf("error execl exchanger\n");
					}
				break;
				default : // parent
					//waiting for  the exchangers thread to start before creating cars
					sigsuspend(&letSigUsr1Mask);
					switch(pid_cars = fork()){
						case -1 :
							printf("error fork cars\n");
						break;
						case 0 : // child
							if(execlp("./cars", "cars", nbCarsString, NULL) == -1){
								printf("error execl cars\n");
							}
						break;
						default : // parent
							
							printf("père mouru\n");
						break;
					}
				break;
			}
		break;
	}
//because every process has executed another main(), only the parent execute the following :

	if(sigaction(SIGINT, &sa_SigStop, NULL) == -1){
		printf("error main.c : main : sigaction\n");
	}
/*******************************************************************				
				  END PREPARATION
********************************************************************/

	waitpid(pid_exchanger, NULL, 0);
	waitpid(pid_server, NULL, 0);
	waitpid(pid_cars, NULL, 0);
	printf("test : %s\n", shExchangers[2].name);
	if(shmdt((void*)shExchangers) == -1){
		printf("error shmdt %d\n", errno);
	}
	if(shmctl(shmId, IPC_RMID, NULL) == -1){
		printf("error shmctl\n");
	}
	if(msgctl(msgqId, IPC_RMID, NULL) == -1){
		printf("error msgctl\n");
	}
	if(sem_destroy(semShExchangers) == -1){
		printf("error sem_destroy\n");
	}
	printf("PROGRAM END : %d\n", nbCars);
	return 0;
}