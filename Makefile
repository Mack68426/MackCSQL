# SQL Server Connection Makefile

CC = gcc
EXE = SqlConnect
OBJS = SqlConnect.c
LIB = -lodbc32

# compile the program with the specified libraries
all: $(OBJS)
<<<<<<< HEAD
	$(CC) $(OBJS) -o $(EXE)  $(LIB)
=======
	$(CC) $(OBJS) -o $(EXE) -l$(LIB) && ./$(EXE)
>>>>>>> 6b95d3c75d3ce2b573bd38cedd473d90ada19350





.PHONY = all