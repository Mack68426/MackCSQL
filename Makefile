# SQL Server Connection Makefile

CC = gcc
EXE = SqlConnect
OBJS = SqlConnect.c
LIB = -lodbc32

# compile the program with the specified libraries
all: $(OBJS)
	$(CC) $(OBJS) -o $(EXE)  $(LIB)





.PHONY = all