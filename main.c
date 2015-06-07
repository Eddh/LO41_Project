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
#include "defs.h"

void* sigusr1_handler(int sig)
{
	return 0;
}

int main(int argc, char *argv[]){
	printf("PROGRAM START\n");

	int pid, pid_exchanger, pid_server, pid_cars;
	int nbCars = NBCARS;
	int shmId = 0;
	char nbCarsString[32];
	key_t key;
	char* input = NULL;
	size_t len = 0, nbCarsStringLen = 32;
	ssize_t read = 0;
	struct Exchanger* shExchangers = NULL;

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

	// key = ftok(argv[0], 'a');
	// msgget(key, 0666 | IPC_CREAT);

	// creating the shared memory segment which will contain all the Exchangers
	if(key = ftok(argv[0], 'a') == -1){
		printf(" error ftok\n");
	}
	shmId = shmget(key, sizeof(struct Exchangers*)*6, 0666 | IPC_CREAT | IPC_EXCL);
	if(shmId == -1){
		printf("error shmget\n");
		shmctl(shmId, IPC_RMID, NULL);
	}
	printf("shmId : %d errno :%d\n", shmId, errno);
	shExchangers = shmat(shmId, (void*)0, 0);
	if(shExchangers == (struct Exchanger*)-1){
		printf("error shmat errno %d %d %s\n", errno, shmId, argv[0]);
	}

	//create4ExchangersCity(shExchangers);

	/*user input*/
	do {
		printf("please enter the number of cars you wish(1 to 1000), nothing->%d : ", NBCARS);
		if(read = getline(&input, &len, stdin) == -1){
		printf("error main.c:main:getline\n");
		}
		nbCars = atoi(input);
	} while(nbCars < 0 || nbCars > 1000);
	if(nbCars == 0){ // default number of cars
		nbCars = NBCARS;
	}
	snprintf(nbCarsString, nbCarsStringLen, "%d", nbCars);



	// creation of all the process
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
							waitpid(pid_exchanger, NULL, 0);
							waitpid(pid_server, NULL, 0);
							waitpid(pid_cars, NULL, 0);
							printf("père mouru\n");
						break;
					}
				break;
			}
		break;
	}
	if(shmdt((void*)shExchangers) == -1){
		printf("error shmdt %d\n", errno);
	}
	if(shmctl(shmId, IPC_RMID, NULL) == -1){
		printf("error shmctl\n");
	}
	
	printf("PROGRAM END : %d\n", nbCars);
	return 0;
}