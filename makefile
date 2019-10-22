CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ =  Loader.o Memory.o Tools.o RegisterFile.o ConditionCodes.o FetchStage.o\
		DecodeStage.o PipeReg.o PipeRegField.o Simulate.o F.o D.o E.o\
		M.o W.o
		
ALLSTGH = F.h D.h E.h M.h W.h
ALLSTGS = FetchStage.h DecodeStage.h ExecuteStage.h MemoryStage.h WritebackStage.h	
USLSUS = Instructions.h RegisterFile.h PipeReg.h PipeRegField.h Status.h
XSTG = RegisterFile.h PipeRegField.h PipeReg.h Stage.h FetchStage.h Status.h\
		Debug.h

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h Loader.h RegisterFile.h ConditionCodes.h PipeReg.h\
		Stage.h Simulate.h

Loader.o: Loader.h Memory.h

Memory.o: Memory.h Tools.h

Tools.o: Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

FetchStage.o: $(XSTG) $(ALLSTGH)

DecodeStage.o: $(XSTG) $(ALLSTGH)

ExecuteStage.o $(XSTG) $(ALLSTGH)

MemoryStage.o: $(XSTG) $(ALLSTGH)

WritebackStage.o: $(XSTG) $(ALLSTGH)

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

Simulate.o: $(ALLSTGH) $(ALLSTGS) Stage.h Simulate.h Memory.h RegisterFile.h\
		ConditionCodes.h
		
F.o: PipeRegField.h PipeReg.h F.h

D.o: $(USLSUS) D.h

E.o: $(USLSUS) E.h

M.o: $(USLSUS) M.h

W.o: $(USLSUS) W.h

clean:
	rm $(OBJ)

run:
	make clean
	make yess
	./run.sh

