CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
LIB = libcalc.so
OBJS = add.o sub.o mul.o div.o

all: main

# Компиляция программы
main: main.o $(LIB)
	$(CC) -o main main.o -L. -lcalc

# Сборка динамической библиотеки
$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)

# Компиляция .o файлов с позиционно-независимым кодом
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so main

