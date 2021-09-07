CC=gcc
CFLAGS=-g
TARGET:test CommandParser/libcli.a
LIBS=-lpthread -L ./CommandParser -lcli -lrt
OBJS=gluethread/glthread.o \
	graph.o \
	net.o \
	nwcli.o \
	topologies.o \
	comm.o

test:testapp.o ${OBJS} CommandParser/libcli.a
	${CC} ${CFLAGS} -z muldefs testapp.o ${OBJS} -o test ${LIBS}

testapp.o:testapp.c
	${CC} ${CFLAGS} -c testapp.c -o testapp.o

gluethread/glthred.o:gluethread/glthread.c
	${CC} ${CFLAGS} -c -I gluethread gluethread/glthread.c -o gluethread/glthread.o

net.o: net.h net.c
	${CC} ${CFLAGS} -c -I . net.c -o net.o

graph.o:graph.c graph.h
	${CC} ${CFLAGS} -c -I . graph.c -o graph.o

topologies.o:topologies.c
	${CC} ${CFLAGS} -c -I . topologies.c -o topologies.o

comm.o:comm.c comm.h
	${CC} ${CFLAGS} -c -I . comm.c -o comm.o

#utils.o:utils.c
#	${CC} ${CFLAGS} -c -I . utils.c -o utils.o

nwcli.o:nwcli.c
	${CC} ${CFLAGS} -c -I . nwcli.c -o nwcli.o

CommandParser/libcli.a:
	(cd CommandParser; make)

clean:
	- rm *.o
	- rm gluethread/glthread.o
	- rm test
	- (cd CommandParser; make clean)

all:
	make
	(cd CommandParser; make)

