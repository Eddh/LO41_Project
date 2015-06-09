/*
 Name : cars.c
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
#include <time.h>
#include "defs.h"

Exchanger* shExchangers;

void * run(void * input){
	int carId = -1, direction = -1, from = -1;
	char *exchangerName = NULL;
	char *fromName = NULL;
	char *dirName = NULL;

	threadCarData* data = (threadCarData*)input;
	carId = data->carId;

	Exchanger* location = getRndExchanger(shExchangers);
	Exchanger* prevLocation = NULL;
	Exchanger* futureLocation = NULL;
	from = getRndEntryRoad(location);
	direction = getRndDirection(location, from);
	futureLocation = getExchanger(shExchangers, location->roads[direction]);

	fromName = location->roadNames[from];
	dirName = location->roadNames[direction];
	
	exchangerName = location->name;
	printf("car number %d created at exchanger %s, entering through %s with direction %s\n", carId, exchangerName, fromName, dirName);
	do {
		printf("car %d on the road %s to crossroad %s\n", carId, fromName, exchangerName);
		// simulating travel
		usleep(1000000);
		printf("car %d arrived at exchanger %s, through road %s with direction %s, verifying if going through is possible\n"
			, carId, exchangerName, fromName, dirName);

		//todo : can the car go through?
		printf("car %d going through exchanger %s, from %s to %s\n", carId, exchangerName, fromName, dirName);
		usleep(200000);
		printf("car %d leaving exchanger %s, through road %s\n", carId, exchangerName, dirName);
		usleep(1000000);
		prevLocation = location;
		
		//// going to new Exchanger
		if(location->roads[direction] != (Exchanger*)(-1)){
			location = getExchanger(shExchangers, location->roads[direction]);
			from = invDirection(direction);
			direction = getRndDirection(location, from);
			fromName = location->roadNames[from];
			dirName = location->roadNames[direction];
			exchangerName = location->name;
		}
		else{
			location = NULL;
		}

	}while(location != NULL);
	printf("car %d leaving the city through road %s of exchanger %s\n", carId, dirName, exchangerName);
	
	return NULL;
}


 int main(int argc, char* argv[]){

 	printf("CARS process\t   START\n");
/*******************************************************************				
				  DECLARATIONS & INITIALIZATIONS
********************************************************************/
 	int nbCars = NBCARS, i = 0;
 	int ppid = getppid();
 
 	key_t key;
	int shmId = -1;
	//// attachment to segment of shared memory containing Exchangers
 	key = ftok("main", 'a');
	if(key == -1){
		printf(" error cars.c:ftok errno : %d\n", errno);
	}
	shmId = shmget(key, sizeof(Exchanger*)*4, 0666);
	if(shmId == -1){
		printf("error cars.c:shmget errno : %d\n", errno);
	}
	shExchangers = (Exchanger*)shmat(shmId, (void*)0, 0);
	if(shExchangers == (Exchanger*)(-1)){
		printf("error cars:shmat errno %d %d\n", errno, shmId);
	}
	printf("test cars.c : %s shmId : %d address : %li\n", shExchangers[2].name, shmId, (uintptr_t)shExchangers);


 	if(argc == 2){
 		nbCars = atoi(argv[1]);
 	}
 	pthread_t *threadCars = malloc(sizeof(pthread_t)*nbCars);
 	threadCarData *carData = malloc(sizeof(threadCarData*)*nbCars);
 	if(threadCars == NULL){
 		printf("error cars.c : main : malloc\n");
 		exit (1);
 	}

/*******************************************************************		
						CAR THREADS CREATION
********************************************************************/		
 	srand(time(0));
 	for(i = 0 ; i < nbCars ; i++){
 		carData[i].carId = i;
 	}
 	for(i = 0 ; i < nbCars ; i++){

 		pthread_create(&threadCars[i], NULL, run, &carData[i]);
 	}


/*******************************************************************		
						PROCESS END PREPARATION
********************************************************************/
 	for(i = 0 ; i < nbCars ; i++){
 		pthread_join(threadCars[i], NULL);
 	}
 	free(threadCars);

 	printf("CARS process\t   END %s\n", argv[1]);

 	return 0;
 }