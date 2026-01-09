CC=gcc
CFLAGS=-O2 -Wall -Wextra -Iinclude
LDFLAGS=
LIBNAME=rawudp

LIB_STATIC=lib$(LIBNAME).a
LIB_SHARED=lib$(LIBNAME).so

LIB_OBJS=src/checksum.o src/packet.o src/client_table.o

SERVER_BIN=echo-server
CLIENT_BIN=echo-client

.PHONY: all clean static shared

all: $(LIB_STATIC) $(LIB_SHARED) $(SERVER_BIN) $(CLIENT_BIN)

static: $(LIB_STATIC)
shared: $(LIB_SHARED)

$(LIB_STATIC): $(LIB_OBJS)
	ar rcs $@ $^

$(LIB_SHARED): CFLAGS += -fPIC
$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared -o $@ $^

src/%.o: src/%.c include/rawudp.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_BIN): server/echo_server.c $(LIB_STATIC) include/rawudp.h
	$(CC) $(CFLAGS) -o $@ server/echo_server.c -L. -l$(LIBNAME)

$(CLIENT_BIN): client/echo_client.c $(LIB_STATIC) include/rawudp.h
	$(CC) $(CFLAGS) -o $@ client/echo_client.c -L. -l$(LIBNAME)

clean:
	rm -f $(LIB_OBJS) $(LIB_STATIC) $(LIB_SHARED) $(SERVER_BIN) $(CLIENT_BIN)
