CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ =  Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o 

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Memory.h RegisterFile.h ConditionCodes.h Loader.h Debug.h PipeReg.h\
		Stage.h Simulate.h

Loader.o: Loader.h Memory.h


Memory.o: Memory.h Tools.h

Tools.o: Tools.h

RegisterFile.o: RegisterFile.h Tools.h


ConditionCodes.o: ConditionCodes.h


clean:
	rm $(OBJ) lab6

run:
	make clean
	make lab7
	./run.sh

