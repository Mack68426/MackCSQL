# SQL Server Connection Makefile

CC = gcc
EXE = SqlConnect.exe
OBJS = SqlConnect.c
LIB = odbc32

# compile the program with the specified libraries
all: $(OBJS)
	$(CC) $(OBJS) -o $(EXE) -l$(LIB) && ./$(EXE)





.PHONY = all