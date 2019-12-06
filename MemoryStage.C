//mem.C
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
#include "Memory.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
/*
 * doClockLow:
 * Performs the Memory stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    Memory * mem = Memory::getInstance();
    bool memError = false;

    uint64_t M_stat = mreg->getstat()->getOutput();
    uint64_t M_icode = mreg->geticode()->getOutput();
    uint64_t M_valE = mreg->getvalE()->getOutput();
    uint64_t M_valA = mreg->getvalA()->getOutput();
    uint64_t M_dstE = mreg->getdstE()->getOutput();
    uint64_t M_dstM = mreg->getdstM()->getOutput();

    uint64_t addr = memAddr(M_icode, M_valE, M_valA);  
    m_valM = 0;
    if(memRead(M_icode))
    {
        m_valM = mem->getLong(addr, memError);
    }
    if(memWrite(M_icode))
    {
        mem->putLong(M_valA, addr, memError);
    }
    
    m_stat = getm_stat(memError, M_stat);

    setWInput(wreg, m_stat, M_icode, M_valE, m_valM, M_dstE, M_dstM);
    return false;
}
/* doClockHigh
 * applies the appropriate control signal to the D
 * and E register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];

    wreg->getstat()->normal();
    wreg->geticode()->normal();
    wreg->getvalE()->normal();
    wreg->getvalM()->normal();
    wreg->getdstE()->normal();
    wreg->getdstM()->normal();
}
/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 */
void MemoryStage::setWInput(W* wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, 
        uint64_t dstE, uint64_t dstM)
{
    wreg->getstat()->setInput(stat);
    wreg->geticode()->setInput(icode);
    wreg->getvalE()->setInput(valE);
    wreg->getvalM()->setInput(valM);
    wreg->getdstE()->setInput(dstE);
    wreg->getdstM()->setInput(dstM);

}

uint64_t MemoryStage::memAddr(uint64_t instr, uint64_t M_valE, uint64_t M_valA)
{
    if(instr == IRMMOVQ || instr == IPUSHQ || instr == ICALL|| instr == IMRMOVQ)
    {
        return M_valE;
    }
    else if (instr == IPOPQ || instr == IRET)
    {
        return M_valA;
    }
    return 0;
}

bool MemoryStage::memRead(uint64_t instr)
{
    return (instr == IMRMOVQ || instr == IPOPQ || instr == IRET);
}

bool MemoryStage::memWrite(uint64_t instr)
{
    return (instr == IRMMOVQ || instr == IPUSHQ || instr == ICALL);
}
uint64_t MemoryStage::getm_stat(bool memError, uint64_t M_stat)
{
    if(memError)
    {
        return SADR;
    }
    return M_stat;
}
