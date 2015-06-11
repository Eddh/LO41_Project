#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include "defs.h"
/*******************************************************************				
				  createExchanger4roads :
				   return a new Exchanger with 4 roads
********************************************************************/
Exchanger createExchanger4roads(){
	Exchanger r;
	strcpy(r.name, "ok");
	int i = 0;

	strcpy(r.roadNames[RIGHT], "RIGHT");
	strcpy(r.roadNames[TOP], "TOP");
	strcpy(r.roadNames[LEFT], "LEFT");
	strcpy(r.roadNames[BOT], "BOT");

	return r;
}
/*******************************************************************				
				  create4ExchangersCity :
				   allocate to the pointer sent 4 Exchangers with 4 roads
********************************************************************/
void create4ExchangersCity(Exchanger* exchangers){
	int i = 0;

	for (i = 0 ; i < 4 ; i++){
		exchangers[i] = createExchanger4roads();
		printf("OK\n");
	}
	strcpy(exchangers[BOT_LEFT].name, "BOTTOM_LEFT");
	strcpy(exchangers[BOT_RIGHT].name, "BOTTOM_RIGHT");
	strcpy(exchangers[TOP_RIGHT].name, "TOP_RIGHT");
	strcpy(exchangers[TOP_LEFT].name, "TOP_LEFT");
	exchangers[BOT_LEFT].roads[RIGHT] = (Exchanger*)((&exchangers[BOT_RIGHT] - exchangers) * sizeof(Exchanger));
	exchangers[BOT_LEFT].roads[TOP] = (Exchanger*)((&exchangers[TOP_LEFT] - exchangers) * sizeof(Exchanger));
	exchangers[BOT_LEFT].roads[LEFT] = (Exchanger*)(-1);
	exchangers[BOT_LEFT].roads[BOT] = (Exchanger*)(-1);

	exchangers[BOT_RIGHT].roads[RIGHT] = (Exchanger*)(-1);
	exchangers[BOT_RIGHT].roads[TOP] = (Exchanger*)((&exchangers[TOP_RIGHT] - exchangers) * sizeof(Exchanger));
	exchangers[BOT_RIGHT].roads[LEFT] = (Exchanger*)((&exchangers[BOT_LEFT] - exchangers) * sizeof(Exchanger));
	exchangers[BOT_RIGHT].roads[BOT] = (Exchanger*)(-1);

	exchangers[TOP_RIGHT].roads[RIGHT] = (Exchanger*)(-1);
	exchangers[TOP_RIGHT].roads[TOP] = (Exchanger*)(-1);
	exchangers[TOP_RIGHT].roads[LEFT] = (Exchanger*)((&exchangers[TOP_LEFT] - exchangers) * sizeof(Exchanger));
	exchangers[TOP_RIGHT].roads[BOT] = (Exchanger*)((&exchangers[BOT_RIGHT] - exchangers) * sizeof(Exchanger));

	exchangers[TOP_LEFT].roads[RIGHT] = (Exchanger*)((&exchangers[TOP_RIGHT] - exchangers) * sizeof(Exchanger));
	exchangers[TOP_LEFT].roads[TOP] = (Exchanger*)(-1);
	exchangers[TOP_LEFT].roads[LEFT] = (Exchanger*)(-1);
	exchangers[TOP_LEFT].roads[BOT] = (Exchanger*)((&exchangers[BOT_LEFT] - exchangers) * sizeof(Exchanger));

	exchangers[BOT_LEFT].exchangerIndex = 0;
	exchangers[BOT_RIGHT].exchangerIndex = 1;
	exchangers[TOP_RIGHT].exchangerIndex = 2;
	exchangers[TOP_LEFT].exchangerIndex = 3;

}
/*******************************************************************
				getRandomExchanger :				
				  return a random Exchanger among 4
********************************************************************/
Exchanger* getRndExchanger(Exchanger *exchangers){
	int rnd = rand()%4;
	return &exchangers[rnd];
}
int getRndDirection(Exchanger* location, int from){
	int rnd = rand()%3;
	if (rnd >= from){
		rnd++;
	}
	return rnd;
}
int getRndEntryRoad(Exchanger* exchanger){
	int rnd = rand()%2, road = -1;
	int i = 0, safety = 0;
	while(road < 0 && safety < 40){
		
		if(i >= 4){
			i = 0;
		}
		if (exchanger->roads[i] == (Exchanger*)(-1)){
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
	if(safety == 40){
		printf("error getRndEntryRoad safety = %d i = %d\n", safety, i);
	}

	return road;
}
int invDirection(int dir){
	if(dir < 2){
		return dir + 2;
	}
	else{
		return dir - 2;
	}

}

int dirAbsPlusRel(int absDir, int relDir){
	int r = absDir + relDir;
	r = r%4;
	if(r < 0){
		r+=4;
	}
	return r;
}
/*******************************************************************
				getExchanger :				
				  add the relative address to the absolute address of a shared memory in order to access an contained Exchanger
********************************************************************/
Exchanger* getExchanger(Exchanger* absAddr, Exchanger* relAddr){
	Exchanger* exchanger = ((Exchanger*)((uintptr_t)(relAddr) + (uintptr_t)absAddr));
	return exchanger;
}

