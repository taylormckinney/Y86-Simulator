CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ =  Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o D.o
ALLSTG = F.h D.h E.h M.h W.h

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

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h $(ALLSTG) Stage.h\
		FetchStage.h Status.h Debug.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h $(ALLSTG) Stage.h\
		DecodeStage.h Status.h Debug.h

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

Simulate.o: $(ALLSTG) Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h\
		FetchStage.h WritebackStage.h Simulate.h Memory.h\
		RegisterFile.h ConditionCodes.h
		
F.o: PipeRegField.h PipeReg.h F.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h

M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h

clean:
	rm $(OBJ) lab6

run:
	make clean
	make lab7
	./run.sh

