
#ifndef DEFS_h
#define DEFS_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
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

typedef struct Exchanger{
	struct Exchanger* roads[4];
	char name[16];
	char roadNames[4][16];
} Exchanger;

typedef struct Car{
	int carId;
	unsigned int locIndex;
	int from, direction;

} Car;

typedef struct CarMsg{
	long mtype;
	Car car;

} CarMsg;

typedef struct AuthMsg{
	long mtype;
	int Authorization;
} AuthMsg;

typedef struct threadCarData{
	int carId;
} threadCarData;

Exchanger createExchanger4roads();
void create4ExchangersCity(Exchanger exchangers[]);
Exchanger* getRndExchanger(Exchanger* exchangers);
int getRndDirection(Exchanger* location, int from);
int getRndEntryRoad(Exchanger* exchanger);
int invDirection(int dir);
int dirAbsPlusRel(int absDir, int relDir);
Exchanger* getExchanger(Exchanger* absAddr, Exchanger* relAddr);

#endif