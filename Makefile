# David Olsen
# dolsen
# 641219

C_OBJ  = client.o
C_SRC  = client.c
C_EXE  = client


S_OBJ  = server.o queue.o mastermind.o
S_SRC  = server.c queue.c mastermind.c
S_EXE  = server

CC     = gcc
CFLAGS = -Wall -O2 -std=gnu99

all: client server

client:   $(C_OBJ) Makefile
	$(CC) $(CFLAGS) -o $(C_EXE) $(C_OBJ)

server:   $(S_OBJ) Makefile
		$(CC) $(CFLAGS) -o $(S_EXE) $(S_OBJ) -lpthread

clean:
	rm -f $(C_OBJ) $(C_EXE) $(S_OBJ) $(S_EXE)

client.o: client.c client.h Makefile

queue.o: queue.c queue.h Makefile
mastermind.o: mastermind.c mastermind.h Makefile

server.o: server.c server.h queue.c queue.h mastermind.c mastermind.h Makefile
