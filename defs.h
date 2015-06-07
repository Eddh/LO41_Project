
#ifndef DEFS_h
#define DEFS_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define NBCARS 10
#define RIGHT 0
#define TOP 1
#define LEFT 2
#define BOT 3

#define BOT_LEFT 0
#define BOT_RIGHT 1
#define TOP_RIGHT 2
#define TOP_LEFT 3

struct Exchanger{
	int nbRoads;
	struct Exchanger** roads;
	char* name;
	char** roadNames;
};

struct threadCarData{
	struct Exchanger* exchangers;
};

struct Exchanger createExchanger4roads();
void create4ExchangersCity(struct Exchanger exchangers[]);
struct Exchanger* getRandomExchanger(struct Exchanger exchangers[], int nbExchangers);
int getEntryRoad(struct Exchanger exchanger);

#endif