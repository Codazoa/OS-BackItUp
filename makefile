CC = gcc
CFLAGS = -Wall -g #-Werror
OBJ = main.o copy.o
HDRS = copy.h options.h

all: backitup

backitup: $(OBJ)
	$(CC) $(CFLAGS) -o backitup $(OBJ) -lpthread

main.o: main.c $(HDRS)
	$(CC) $(CFLAGS) -c main.c

copy.o: copy.c $(HDRS)
	$(CC) $(CFLAGS) -c copy.c

clean:
	rm -f $(OBJ) backitup