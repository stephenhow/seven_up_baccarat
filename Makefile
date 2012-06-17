OBJS = main.o baccarat7up.o mersenne64.o averager.o
CC = g++
CPPFLAGS = -Wall -O3
CFLAGS = -Wall -O3 -c
LFLAGS = -Wall $(DEBUG)

sevenUp : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o sevenUp