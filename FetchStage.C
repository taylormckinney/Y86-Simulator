#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Memory.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
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
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    Memory * mem = Memory::getInstance();

    uint64_t f_icode = 0, f_ifun = 0, f_valC = 0, f_valP = 0;
    uint64_t f_rA = RNONE, f_rB = RNONE, f_stat = SAOK;

    f_pc = selectPC(freg, mreg, wreg);

    uint64_t mem_icode = mem->getByte(f_pc, memError);
    mem_icode = Tools::getBits((uint64_t)mem_icode, 4, 7);
    f_icode = getf_icode(memError, mem_icode); 

    uint64_t mem_ifun = mem->getByte(f_pc, memError);
    mem_ifun = Tools::getBits((uint64_t)mem_ifun, 0, 3);
    f_ifun = getf_ifun(memError, mem_ifun); 

    bool needsIds = FetchStage::needRegIds(f_icode);
    bool needsValC = FetchStage::needValC(f_icode);

    f_valP = PCincrement(f_pc, needsIds, needsValC);
    if(needsIds)
    {
        getRegIds(mem, f_rA, f_rB);
    }
    if(needsValC)
    {
        f_valC = getValC(mem, f_icode);
    }

    freg->getpredPC()->setInput(predictPC(f_icode, f_valC, f_valP));

    f_stat = getf_stat(f_icode, memError);

    DecodeStage * dstage = (DecodeStage *)stages[DSTAGE];
    ExecuteStage * estage = (ExecuteStage *)stages[ESTAGE];
    calculateControlSignals(ereg, dstage, estage);

    //provide the input values for the D register
    setDInput(dreg, f_stat, f_icode, f_ifun, f_rA, f_rB, f_valC, f_valP);
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
    if(F_stall)
    {
        freg->getpredPC()->stall();
    }
    else
    {
        freg->getpredPC()->normal();
    }

    if(D_stall)
    {
        dreg->getstat()->stall();
        dreg->geticode()->stall();
        dreg->getifun()->stall();
        dreg->getrA()->stall();
        dreg->getrB()->stall();
        dreg->getvalC()->stall();
        dreg->getvalP()->stall();
    }
    else if(D_bubble)
    {
        dreg->getstat()->bubble(SAOK);
        dreg->geticode()->bubble(INOP);
        dreg->getifun()->bubble();
        dreg->getrA()->bubble(RNONE);
        dreg->getrB()->bubble(RNONE);
        dreg->getvalC()->bubble();
        dreg->getvalP()->bubble();
    }
    else
    {
        dreg->getstat()->normal();
        dreg->geticode()->normal();
        dreg->getifun()->normal();
        dreg->getrA()->normal();
        dreg->getrB()->normal();
        dreg->getvalC()->normal();
        dreg->getvalP()->normal();
    }

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
    if(needsIds && !needsValC)
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
    return f_pc +1;
}

void FetchStage::getRegIds(Memory * mem, uint64_t& rA, uint64_t& rB)
{
    uint64_t idsByte = mem->getByte(f_pc+1, memError);
    rA = Tools::getBits(idsByte, 4, 7);
    rB =Tools::getBits(idsByte, 0, 3);
}

uint64_t FetchStage::getValC(Memory * mem, uint64_t f_icode)
{
    uint64_t addr = f_pc + 2;
    if(f_icode == IJXX || f_icode == ICALL)
    { 
        addr = f_pc+1;
    }
    uint8_t bytes[8];
    for(int i=0; i <8; i++)
    {
        bytes[i] = mem->getByte(addr+i, memError);
    }
    return Tools::buildLong(bytes);
}

bool FetchStage::instrValid(uint64_t f_icode)
{
    return (f_icode >= IHALT && f_icode <= IPOPQ); 
}

uint64_t FetchStage::getf_icode(bool memError, uint64_t mem_icode)
{
    if(memError)
    {
        return INOP;
    }
    return mem_icode;
}

uint64_t FetchStage::getf_ifun(bool memError, uint64_t mem_ifun)
{
    if(memError)
    {
        return INOP;
    }
    return mem_ifun;
}

uint64_t FetchStage::getf_stat(uint64_t f_icode, bool memError)
{
    if(memError)
    {
        return SADR;
    }
    if(!instrValid(f_icode))
    {
        return SINS;
    }
    if(f_icode == IHALT)
    {
        return SHLT;
    }
    return SAOK;
}

bool FetchStage::getf_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB)
{
    return ( (E_icode == IMRMOVQ || E_icode == IPOPQ) && 
            (E_dstM == d_srcA || E_dstM == d_srcB) );
}

bool FetchStage::getd_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB)
{
    return ( (E_icode == IMRMOVQ || E_icode == IPOPQ) && 
            (E_dstM == d_srcA || E_dstM == d_srcB) );
}

void FetchStage::calculateControlSignals(E * ereg, DecodeStage * dstage, ExecuteStage * estage)
{
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t e_Cnd = estage->gete_Cnd();
    uint64_t d_srcA = dstage->getd_srcA();
    uint64_t d_srcB = dstage->getd_srcB();

    F_stall = getf_stall(E_icode, E_dstM, d_srcA, d_srcB);
    D_stall = getd_stall(E_icode, E_dstM, d_srcA, d_srcB);
    D_bubble = getd_bubble(E_icode, e_Cnd);
}

bool FetchStage::getd_bubble(uint64_t E_icode,uint64_t e_Cnd)
{
    return (E_icode == IJXX && !e_Cnd);
}
