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

 int main(int argc, char* argv[]){

 	printf("CARS process\t START\n");
 	printf("CARS process\t END\n");

 	return 0;
 }