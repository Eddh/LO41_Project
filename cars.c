/*
 Name : cars.c
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
#include <time.h>
#include "defs.h"

void * run(void * input){
	int numCar = -1, exchanger = -1, direction = -1, from = -1;
	char *exchangerName = NULL;
	char *roadName = NULL;

	memcpy(&numCar, &input, sizeof(int));

	
	exchanger = rand() % 4;
	from = rand()%2;
	direction = rand() % 3;
	
	switch(exchanger){
		case BOT_LEFT :
			exchangerName = "BOTTOM_LEFT";
		break;
		case BOT_RIGHT :
			exchangerName = "BOTTOM_RIGHT";
		break;
		case TOP_RIGHT :
			exchangerName = "TOP_RIGHT";
		break;
		case TOP_LEFT :
			exchangerName = "TOP_LEFT";
		break;
		default : // error
			printf("error cars.c:run:switch_exchanger\n");
		break;
	}

	printf("car number %d created at exchanger %s, with direction %d\n", numCar, exchangerName, direction);
	return NULL;
}


 int main(int argc, char* argv[]){

 	printf("CARS process\t   START\n");

 	int nbCars = NBCARS, i = 0;
 	int ppid = getppid();
 	pthread_t *threadCars = malloc(sizeof(pthread_t)*nbCars);

 	if(argc == 2){
 		nbCars = atoi(argv[1]);
 	}

 	if(threadCars == NULL){
 		printf("error cars.c : main : malloc\n");
 		exit (1);
 	}

 	srand(time(0));
 	for(i = 0 ; i < nbCars ; i++){
 		pthread_create(&threadCars[i], NULL, run, (void*)i);
 	}

 	for(i = 0 ; i < nbCars ; i++){
 		pthread_join(threadCars[i], NULL);
 	}
 	free(threadCars);

 	printf("CARS process\t   END %s\n", argv[1]);

 	return 0;
 }