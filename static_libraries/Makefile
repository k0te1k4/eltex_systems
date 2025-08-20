CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2
LIB = libcalc.a
OBJS = add.o sub.o mul.o div.o

all: main

main: main.o $(LIB)
	$(CC) $(CFLAGS) -o main main.o -L. -lcalc

$(LIB): $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a main

