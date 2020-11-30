CC		=gcc
LIB		=-lpthread -lsqlite3
INCS	=$(wildcard *.h)

TOP_PATH = $(shell pwd)
LIB_PATH = $(TOP_PATH)/sqlite3lib/

C_OBJ	=$(wildcard *.c)
OBJ		=$(patsubst %.c,%.o,$(C_OBJ))

tcp_server:$(OBJ)
	$(CC) -g -L $(LIB_PATH) -o tcp_server $(OBJ) $(LIB)

OBJ:$(INC)

.PHONY:clean
clean:
	rm tcp_server
	rm $(OBJ)
