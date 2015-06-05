
name = main
#object files
objects = $(name).o exchanger.o server.o cars.o

#default target
all : $(name) exchanger server cars
	
$(name) :
exchanger :
server :
cars :

#cars : $(objects)
#	cc -o cars $(objects)





run : all $(name)
	./$(name)



.PHONY : clean
clean :
	rm $(name) $(objects)
	