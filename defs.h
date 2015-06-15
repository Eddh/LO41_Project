
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

#define EAST 0
#define NORTH 1
#define WEST 2
#define SOUTH 3

#define BOT_LEFT 0
#define BOT_RIGHT 1
#define TOP_RIGHT 2
#define TOP_LEFT 3

#define ENTERING 0
#define WAITING 1
#define CROSSING 2
#define LEAVING 3



typedef struct Exchanger{
	struct Exchanger* roads[4];
	char name[16];
	char roadNames[4][16];
	int exchangerIndex;
	int msgqId;
} Exchanger;

typedef struct Car{
	int carId;
	unsigned int locIndex;
	int from, direction;
} Car;

typedef struct CarMsg{
	long mtype;
	int carId;
	unsigned int locIndex;
	int from, direction;
	int action;// ENTERING 0, WAITING 1, CROSSING 2, LEAVING 3 
} CarMsg;

typedef struct AuthAsk{
	long mtype;
	int carId;
	int from, direction;
} AuthAsk;
typedef struct AuthAns{
	long mtype;
	int Authorization;
} AuthAns;
typedef struct EndMsg{
	long mtype;
} EndMsg;

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