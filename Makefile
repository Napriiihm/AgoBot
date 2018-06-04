CC = gcc
CFLAGS = -std=c99 -g
LDFLAGS = -lpthread -lm -lwebsockets
EXEC = AgoBot

all : $(EXEC)

$(EXEC) : main.o IA.o Utils.o
	$(CC) -o $(EXEC) *.o $(LDFLAGS)

main.o : main.c
	$(CC) -o $@ -c $< $(CFLAGS)

IA.o : IA.c
	$(CC) -o $@ -c $< $(CFLAGS)

Utils.o : Utils.c
	$(CC) -o $@ -c $< $(CFLAGS)
