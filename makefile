
name = main
#object files
objects = $(name).o exchanger.o server.o cars.o defs.o

deps = defs.o
#libraries
libs = -pthread

#default target
all :  $(name) exchanger server cars 
	
$(name) : $(name).o
	gcc $(libs) -o $(name) $(name).o $(deps)
exchanger : exchanger.o
	gcc $(libs) -o exchanger exchanger.o $(deps)
server : server.o
	gcc $(libs) -o server server.o
cars : cars.o
	gcc $(libs) -o cars cars.o $(deps)

$(name).o : $(name).c defs.o defs.h
	gcc -c -o $(name).o $(name).c
exchanger.o : exchanger.c defs.h
	gcc -c -o exchanger.o exchanger.c
server.o : server.c defs.h
	gcc -c -o server.o server.c
cars.o : cars.c defs.h
	gcc -c -o cars.o cars.c
defs.o : defs.c defs.h
	gcc -c -o defs.o defs.c

#cars : $(objects)
#	cc -o cars $(objects)





run : all $(name)
	./$(name)

.PHONY : clean mrproper
clean :
	rm $(name) $(objects)
mrproper : clean
	rm cars exchanger server
	