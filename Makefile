#    Name: Robert Choate

# This makefile supports separate compilation vs compiling all at once
CC=g++
CFLAGS=-std=c++0x -g
LIBS=-lpthread -lrt

mizzo: mizzo.o producer.o consumer.o
	$(CC) $(CFLAGS) -o mizzo mizzo.o producer.o consumer.o $(LIBS)

mizzo.o: mizzo.cpp
	$(CC) -c $(CFLAGS) mizzo.cpp
#Self note: -c is for compiling only, we don't need to link until the command above

producer.o: producer.cpp
	$(CC) -c $(CFLAGS) producer.cpp

consumer.o: consumer.cpp
	$(CC) -c $(CFLAGS) consumer.cpp


clean:
	rm mizzo *.o