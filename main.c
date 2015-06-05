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

int main(int argc, char *argv[]){
	int pid, pid_exchanger, pid_server, pid_cars;
	key_t key;

	printf("Program start\n");
	key = ftok(argv[0], 'a');
	msgget(key, 0666 | IPC_CREAT);

	// creation of all the process
	switch(pid_server = fork()){
		case -1 :
			printf("error fork server\n");
		break;
		case 0 :
			if(execl("./server", "server", NULL) == -1){
				printf("error execl server\n");
			}
		break;
		default :
			printf("son père\n");
			switch(pid_exchanger = fork()){
				case -1 :

					printf("error fork exchanger\n");
				break;
				case 0 :
					if(execl("./exchanger", "exchanger", NULL) == -1){
						printf("error execl exchanger\n");
					}
				break;
				default :
					switch(pid_cars = fork()){
						case -1 :
							printf("error fork cars\n");
						break;
						case 0 :
							if(execl("./cars", "cars", NULL)){
								printf("error execl cars\n");
							}
						break;
						default :
							printf("son père\n");
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
	
	

	return 0;
}