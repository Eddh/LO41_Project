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
int msgqId;
// mutexDir and dirAvailable initialized in main function
pthread_mutex_t mutexDir[4][4] = {PTHREAD_MUTEX_INITIALIZER};
int dirAvailable[4][4];
/*******************************************************************				
				  		THREAD FUNCTION
********************************************************************/
void * run(void * input){
	int carId = -1, direction = -1, from = -1;
	char *exchangerName = NULL;
	char *fromName = NULL;
	char *dirName = NULL;
	int msgqIdLocal = 0;
	int locIndex = 0;
	threadCarData* data = (threadCarData*)input;
	carId = data->carId;

	Exchanger* location = getRndExchanger(shExchangers);
	Exchanger* prevLocation = NULL;
	Exchanger* futureLocation = NULL;
	from = getRndEntryRoad(location);
	direction = getRndDirection(location, from);
	futureLocation = getExchanger(shExchangers, location->roads[direction]);

	msgqIdLocal = location->msgqId;
	fromName = location->roadNames[from];
	dirName = location->roadNames[direction];
	locIndex = location->exchangerIndex;
	
	exchangerName = location->name;
	Car thisCar;
	

	CarMsg carMsg;
	carMsg.mtype = 1;
	carMsg.carId = carId;
	carMsg.locIndex = location->exchangerIndex;
	carMsg.from = from;
	carMsg.direction = direction;
	carMsg.action = ENTERING;

	AuthAsk authAsk;
	authAsk.mtype = 2;
	authAsk.carId = carId;
	authAsk.from = from;
	authAsk.direction = direction;

	printf("car number %d created at exchanger %s, entering through %s with direction %s\n", carId, exchangerName, fromName, dirName);
/*******************************************************************				
				  			CAR LOOP
********************************************************************/
	do {
		msgsnd(msgqIdLocal, &carMsg, sizeof(CarMsg) - sizeof(long), 0);
		printf("car %d on the road %s to crossroad %s\n", carId, fromName, exchangerName);
		// simulating travel
		usleep(1000000);
		printf("car %d arrived at exchanger %s, through road %s with direction %s, verifying availability\n"
			, carId, exchangerName, fromName, dirName);

		//todo : can the car go through?
		authAsk.mtype = 2;
		authAsk.carId = carId;
		authAsk.from = from;
		authAsk.direction = direction;
		msgsnd(msgqIdLocal, &authAsk, sizeof(AuthAsk) - sizeof(long), 0);

		// checking availability of the road in order to avoid collisions, and then indicating others car if the road is clear thanks to booleans, protected by a mutex each
		pthread_mutex_lock(&mutexDir[locIndex][EAST]); pthread_mutex_lock(&mutexDir[locIndex][NORTH]); pthread_mutex_lock(&mutexDir[locIndex][WEST]); pthread_mutex_lock(&mutexDir[locIndex][SOUTH]);

		if(direction == invDirection(from)){ // if the car is going straight ahead, block only the perpendicular direction
			if(dirAvailable[locIndex][direction]){
				dirAvailable[locIndex][(direction + 1)%4] = 0;
				dirAvailable[locIndex][(direction + 3)%4] = 0;
				printf("car %d : road clear, going to cross exchanger %s \n", carId, exchangerName);
			}
			else{
				// active wait
				printf("car %d  : can't cross, %d %d %d %d waiting\n", carId, dirAvailable[locIndex][0], dirAvailable[locIndex][1], dirAvailable[locIndex][2], dirAvailable[locIndex][3]);
				do{
					pthread_mutex_unlock(&mutexDir[locIndex][EAST]); pthread_mutex_unlock(&mutexDir[locIndex][NORTH]); pthread_mutex_unlock(&mutexDir[locIndex][WEST]); pthread_mutex_unlock(&mutexDir[locIndex][SOUTH]);
					usleep(50000);
					pthread_mutex_lock(&mutexDir[locIndex][EAST]); pthread_mutex_lock(&mutexDir[locIndex][NORTH]); pthread_mutex_lock(&mutexDir[locIndex][WEST]); pthread_mutex_lock(&mutexDir[locIndex][SOUTH]);
				}while(!dirAvailable[locIndex][direction]);
				printf("car %d  : road liberated, going to cross exchanger\n", carId);
				dirAvailable[locIndex][(direction + 1)%4] = 0;
				dirAvailable[locIndex][(direction + 3)%4] = 0;
			}
		}
		else if(direction == (from + 3)%4){ // if the car is turning to the left, block every direction
			if(dirAvailable[locIndex][direction] && dirAvailable[locIndex][(direction + 1)%4] && dirAvailable[locIndex][(direction + 2)%4] && dirAvailable[locIndex][(direction + 3)%4]){
				dirAvailable[locIndex][0] = 0;
				dirAvailable[locIndex][1] = 0;
				dirAvailable[locIndex][2] = 0;
				dirAvailable[locIndex][3] = 0;
				printf("car %d : road clear, going to cross exchanger %s \n", carId, exchangerName);
			}
			else{
				// active wait
				printf("car %d  : can't cross, %d %d %d %d waiting\n", carId, dirAvailable[locIndex][0], dirAvailable[locIndex][1], dirAvailable[locIndex][2], dirAvailable[locIndex][3]);
				do{
					pthread_mutex_unlock(&mutexDir[locIndex][EAST]); pthread_mutex_unlock(&mutexDir[locIndex][NORTH]); pthread_mutex_unlock(&mutexDir[locIndex][WEST]); pthread_mutex_unlock(&mutexDir[locIndex][SOUTH]);
					usleep(50000);
					pthread_mutex_lock(&mutexDir[locIndex][EAST]); pthread_mutex_lock(&mutexDir[locIndex][NORTH]); pthread_mutex_lock(&mutexDir[locIndex][WEST]); pthread_mutex_lock(&mutexDir[locIndex][SOUTH]);
				}while(!(dirAvailable[locIndex][direction] && dirAvailable[locIndex][(direction + 1)%4] && dirAvailable[locIndex][(direction + 2)%4] && dirAvailable[locIndex][(direction + 3)%4]));
				printf("car %d  : road liberated, going to cross exchanger\n", carId);
				dirAvailable[locIndex][0] = 0;
				dirAvailable[locIndex][1] = 0;
				dirAvailable[locIndex][2] = 0;
				dirAvailable[locIndex][3] = 0;
			}

		}
		else if(direction == (from + 1)%4){ // if the car is turning to the right, block the direction and the left relative to the direction
			if(dirAvailable[locIndex][direction] && dirAvailable[locIndex][(direction + 3)%4] && dirAvailable[locIndex][(direction + 1)%4]){
				dirAvailable[locIndex][direction] = 0;
				dirAvailable[locIndex][(direction + 1)%4] = 0;
				printf("car %d : road clear, going to cross exchanger %s \n", carId, exchangerName);
			}
			else{
				// active wait
				printf("car %d  : can't cross, %d %d %d %d waiting\n", carId, dirAvailable[locIndex][0], dirAvailable[locIndex][1], dirAvailable[locIndex][2], dirAvailable[locIndex][3]);
				do{
					pthread_mutex_unlock(&mutexDir[locIndex][EAST]); pthread_mutex_unlock(&mutexDir[locIndex][NORTH]); pthread_mutex_unlock(&mutexDir[locIndex][WEST]); pthread_mutex_unlock(&mutexDir[locIndex][SOUTH]);
					usleep(50000);
					pthread_mutex_lock(&mutexDir[locIndex][EAST]); pthread_mutex_lock(&mutexDir[locIndex][NORTH]); pthread_mutex_lock(&mutexDir[locIndex][WEST]); pthread_mutex_lock(&mutexDir[locIndex][SOUTH]);
				}while(!(dirAvailable[locIndex][direction] && dirAvailable[locIndex][(direction + 3)%4] && dirAvailable[locIndex][(direction + 1)%4]));
				printf("car %d  : road liberated, going to cross exchanger\n", carId);
				dirAvailable[locIndex][direction] = 0;
				dirAvailable[locIndex][(direction + 1)%4] = 0;
			}
		}
		printf(" booleans after entering : E %d N %d W %d S %d \n", dirAvailable[locIndex][0], dirAvailable[locIndex][1], dirAvailable[locIndex][2], dirAvailable[locIndex][3]);
		pthread_mutex_unlock(&mutexDir[locIndex][EAST]); pthread_mutex_unlock(&mutexDir[locIndex][NORTH]); pthread_mutex_unlock(&mutexDir[locIndex][WEST]); pthread_mutex_unlock(&mutexDir[locIndex][SOUTH]);

		carMsg.action = CROSSING;
		msgsnd(msgqIdLocal, &carMsg, sizeof(carMsg) - sizeof(long), 0);

		printf("car %d going through exchanger %s, from %s to %s\n", carId, exchangerName, fromName, dirName);
		usleep(200000);

		// liberating the road in order to let other cars go through
		pthread_mutex_lock(&mutexDir[locIndex][EAST]); pthread_mutex_lock(&mutexDir[locIndex][NORTH]); pthread_mutex_lock(&mutexDir[locIndex][WEST]); pthread_mutex_lock(&mutexDir[locIndex][SOUTH]);
		if(direction == invDirection(from)){ // if the car is going straight ahead, block only the perpendicular direction
			dirAvailable[locIndex][(direction + 1)%4] = 1;
			dirAvailable[locIndex][(direction + 3)%4] = 1;
			printf("car %d : leaving the crossing %s, liberating the directions %d and %d \n", carId, exchangerName, (direction + 1)%4, (direction + 3)%4);
		}
		else if(direction == (from + 3)%4){ // if the car is turning to the left, block every direction
			dirAvailable[locIndex][0] = 1;
			dirAvailable[locIndex][1] = 1;
			dirAvailable[locIndex][2] = 1;
			dirAvailable[locIndex][3] = 1;
			printf("car %d : leaving the crossing %s, liberating all directions \n", carId, exchangerName);
		}
		else if(direction == (from + 1)%4){ // if the car is turning to the right, block the direction and the left relative to the direction
			dirAvailable[locIndex][direction] = 1;
			dirAvailable[locIndex][(direction + 1)%4] = 1;
			printf("car %d : leaving the crossing %s, liberating directions %d and %d \n", carId, exchangerName, direction, (direction + 1)%4);
		}
		printf(" booleans after %d crossed : E %d N %d W %d S %d \n", carId, dirAvailable[locIndex][0], dirAvailable[locIndex][1], dirAvailable[locIndex][2], dirAvailable[locIndex][3]);
		pthread_mutex_unlock(&mutexDir[locIndex][EAST]); pthread_mutex_unlock(&mutexDir[locIndex][NORTH]); pthread_mutex_unlock(&mutexDir[locIndex][WEST]); pthread_mutex_unlock(&mutexDir[locIndex][SOUTH]);

		carMsg.action = LEAVING;
		msgsnd(msgqIdLocal, &carMsg, sizeof(carMsg) - sizeof(long), 0);
		printf("car %d leaving exchanger %s, through road %s\n", carId, exchangerName, dirName);
		usleep(1000000);
		prevLocation = location;
		msgsnd(msgqIdLocal, &carMsg, sizeof(AuthAsk) - sizeof(long), 0);
		//// going to new Exchanger
		if(location->roads[direction] != (Exchanger*)(-1)){
			location = getExchanger(shExchangers, location->roads[direction]);
			from = invDirection(direction);
			direction = getRndDirection(location, from);
			fromName = location->roadNames[from];
			dirName = location->roadNames[direction];
			exchangerName = location->name;
			msgqIdLocal = location->msgqId;

			carMsg.locIndex = location->exchangerIndex;
			carMsg.from = from;
			carMsg.direction = direction;
			carMsg.action = ENTERING;
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
 	int nbCars = NBCARS, i = 0, j = 0;
 	int ppid = getppid();
 	for(i = 0 ; i < 4 ; i++){
 		for(j = 0 ; j < 4 ; j++){
 			// mutexDir[i][j] = PTHREAD_MUTEX_INITIALIZER;
 			dirAvailable[i][j] = 1;
 		}
 	}
 	key_t shmKey, msgKey;
	int shmId = -1;
	//// attachment to segment of shared memory containing Exchangers
 	shmKey = ftok("main", 'a');
	if(shmKey == -1){
		printf(" error cars.c:ftok errno : %d\n", errno);
	}
	shmId = shmget(shmKey, sizeof(Exchanger*)*4, 0666);
	if(shmId == -1){
		printf("error cars.c:shmget errno : %d\n", errno);
	}
	shExchangers = (Exchanger*)shmat(shmId, (void*)0, 0);
	if(shExchangers == (Exchanger*)(-1)){
		printf("error cars:shmat errno %d %d\n", errno, shmId);
	}

	msgKey = ftok("main", 'a');
	if(msgKey == -1){
		printf("error cars.c:ftok errno %d\n", errno);
	}
	msgqId = msgget(msgKey, 0666 | IPC_CREAT);
	if(msgqId == -1){
		printf("error cars.c:msgget errno%d\n", errno);
	}

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