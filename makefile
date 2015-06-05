
name = main
#object files
objects = $(name).o exchanger.o server.o cars.o

#libraries
libs = -pthread

#default target
all : $(name) exchanger server cars
	
$(name) : $(name).o
	gcc $(libs) -o $(name) $(name).o
exchanger : exchanger.o
	gcc $(libs) -o exchanger exchanger.o
server : server.o
	gcc $(libs) -o server server.o
cars : cars.o
	gcc $(libs) -o cars cars.o

$(name).o : $(name).c
	gcc -c -o $(name).o $(name).c
exchanger.o : exchanger.c
	gcc -c -o exchanger.o exchanger.c
server.o : server.c
	gcc -c -o server.o server.c
cars.o : cars.c
	gcc -c -o cars.o cars.c

#cars : $(objects)
#	cc -o cars $(objects)





run : all $(name)
	./$(name)



.PHONY : clean
clean :
	rm $(name) $(objects)
	