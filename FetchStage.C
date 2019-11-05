#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"
#include "Tools.h"
 /* Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   
   Memory * mem = Memory::getInstance();
   
   uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
   bool error = 0;
   
   f_pc = selectPC(freg, mreg, wreg);
   
   icode = mem->getByte(f_pc, error);
   icode = Tools::getBits((uint64_t)icode, 4, 7);
   ifun = mem->getByte(f_pc, error) & 0x1; 
   
   bool needsIds = FetchStage::needRegIds(icode);
   bool needsValC = FetchStage::needValC(icode);

   valP = PCincrement(f_pc, needsIds, needsValC);
   if(needsIds)
   {
       rA =Tools::getBits(mem->getByte(f_pc + 1, error), 1, 1); 
       rB =Tools::getBits(mem->getByte(f_pc+1, error), 0, 0);
   }
   if(needsValC)
   {
       if(icode == IJXX || icode == ICALL)
       {
           valC = mem->getLong(f_pc+1, error);
       }
       valC = mem->getLong(f_pc +2, error);
   }

   freg->getpredPC()->setInput(predictPC(icode, valC, valP));

   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];

   freg->getpredPC()->normal();
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}

uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
    uint64_t M_icode = mreg->geticode()->getOutput();
    uint64_t M_Cnd = mreg->getCnd()->getOutput();
    uint64_t M_valA = mreg->getvalA()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();
    uint64_t W_icode = wreg->geticode()->getOutput();
    uint64_t F_predPC = freg->getpredPC()->getOutput();
    if(M_icode == IJXX && !M_Cnd)
    {
        return M_valA;
    }
    if(W_icode == IRET)
    {
        return W_valM;
    }

    return F_predPC;
}

uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
    if(f_icode == IJXX || f_icode == ICALL)
    {
        return f_valC;
    }
    return f_valP;
}

bool FetchStage::needRegIds(uint64_t f_icode)
{
    return (f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ || f_icode == IPOPQ
        || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ);
}

bool FetchStage::needValC(uint64_t f_icode)
{
    return (f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IJXX
        || f_icode == ICALL);
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needsIds, bool needsValC)
{
    if(!needsIds && !needsValC)
    {
        return f_pc +1;
    }
    else if(needsIds && !needsValC)
    {
        return f_pc+2;
    }
    else if(!needsIds && needsValC)
    {
        return f_pc +9;
    }
    else if (needsIds && needsValC)
    {
        return f_pc + 10;
    }
}
