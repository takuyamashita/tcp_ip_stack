CC=gcc
CFLAGS=-g
TARGET:test
LIBS=-lpthread -L ./CommandParser -lcli
OBJS=gluethread/glthread.o \
	graph.o \
	net.o \
	topologies.o

test:testapp.o ${OBJS} CommandParser/libcli.a
	${CC} ${CFLAGS} testapp.o ${OBJS} -o test ${LIBS}

testapp.o:testapp.c
	${CC} ${CFLAGS} -c testapp.c -o testapp.o

gluethread/glthred.o:gluethread/glthread.c
	${CC} ${CFLAGS} -c -I gluethread gluethread/glthread.c -o gluethread/glthread.o

net.o: net.h net.c
	${CC} ${CFLAGS} -c -I . net.c -o net.o

graph.o:graph.c graph.h
	${CC} ${CFLAGS} -c -I . graph.c -o graph.o

topologies.c:topologies.c
	${CC} ${CFLAGS} -c -I . topologies.c -o topologies.o

CommandParser/libcli.a:
	(cd CommandParser; make)

clean:
	rm *.o
	rm gluethread/glthread.o
	rm test
	(cd CommandParser; make clean)

all:
	make
	(cd CommandParser; make)

