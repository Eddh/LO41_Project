#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "defs.h"

struct Exchanger createExchanger4roads(){
	struct Exchanger r;
	r.nbRoads = 4;
	r.roads = malloc(sizeof(struct Exchanger**)*4);
	r.name = NULL;
	int i = 0;
	for(i = 0 ; i < 4 ; i++){
		r.roads[i] = NULL;
	}

	r.roadNames = malloc(sizeof(char*)*4);
	for(i = 0 ; i < 4 ; i++){
		r.roadNames[i] = malloc(sizeof(char)*32);
	}

	r.roadNames[0] = "BOT_LEFT";
	r.roadNames[1] = "BOT_RIGHT";
	r.roadNames[2] = "TOP_RIGHT";
	r.roadNames[3] = "TOP_LEFT";
	return r;
}

void create4ExchangersCity(struct Exchanger exchangers[]){
	int i = 0;

	// free(exchangers);

	//exchangers = malloc(sizeof(struct Exchanger*)*4);

	for (i = 0 ; i < 4 ; i++){
		exchangers[i] = createExchanger4roads();
		printf("OK\n");
	}
	
	exchangers[BOT_LEFT].name = "BOTTOM_LEFT";
	exchangers[BOT_RIGHT].name = "BOTTOM_RIGHT";
	exchangers[TOP_RIGHT].name = "TOP_RIGHT";
	exchangers[TOP_LEFT].name = "TOP_LEFT";
	
	exchangers[BOT_LEFT].roads[RIGHT] = &exchangers[BOT_RIGHT];
	exchangers[BOT_LEFT].roads[TOP] = &exchangers[TOP_LEFT];
	exchangers[BOT_LEFT].roads[LEFT] = NULL;
	exchangers[BOT_LEFT].roads[BOT] = NULL;

	exchangers[BOT_RIGHT].roads[RIGHT] = NULL;
	exchangers[BOT_RIGHT].roads[TOP] = &exchangers[TOP_RIGHT];
	exchangers[BOT_RIGHT].roads[LEFT] = &exchangers[BOT_LEFT];
	exchangers[BOT_RIGHT].roads[BOT] = NULL;

	exchangers[TOP_RIGHT].roads[RIGHT] = NULL;
	exchangers[TOP_RIGHT].roads[TOP] = NULL;
	exchangers[TOP_RIGHT].roads[LEFT] = &exchangers[TOP_LEFT];
	exchangers[TOP_RIGHT].roads[BOT] = &exchangers[BOT_RIGHT];

	exchangers[TOP_LEFT].roads[RIGHT] = &exchangers[TOP_RIGHT];
	exchangers[TOP_LEFT].roads[TOP] = NULL;
	exchangers[TOP_LEFT].roads[LEFT] = NULL;
	exchangers[TOP_LEFT].roads[BOT] = &exchangers[BOT_LEFT];

}

struct Exchanger* getRandomExchanger(struct Exchanger exchangers[], int nbExchangers){
	int rnd = rand()%nbExchangers;
	return &exchangers[rnd];
}

int getEntryRoad(struct Exchanger exchanger){
	int rnd = rand()%2, road = -1;
	int i = 0, safety = 0;
	while(road < 0 && safety < 40){
		
		if(i >= exchanger.nbRoads){
			i = 0;
		}
		if (exchanger.roads[i] == NULL){
			if(rnd == 0){
				road = i;
			}
			else{
				rnd--;
			}
		}
		i++;
		safety++;
	}
	if(safety = 40){
		printf("error input getEntryRoad\n");
	}
	return road;
}