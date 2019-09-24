CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ =  Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o lab6.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

lab6: $(OBJ)

lab6.o: Memory.h RegisterFile.h ConditionCodes.h Loader.h

Loader.o: Loader.h Memory.h


Memory.o: Memory.h Tools.h

Tools.o: Tools.h

RegisterFile.o: RegisterFile.h Tools.h


ConditionCodes.o: ConditionCodes.h ConditionCodesTester.h


clean:
	rm $(OBJ) lab6

run:
	make clean
	make lab6
	./run.sh

