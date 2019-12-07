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
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];

    uint64_t D_stat = dreg->getstat()->getOutput();
    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t D_ifun = dreg->getifun()->getOutput();
    uint64_t D_valC = dreg->getvalC()->getOutput();
    uint64_t D_rA = dreg->getrA()->getOutput();
    uint64_t D_rB = dreg->getrB()->getOutput();

    d_srcA = getSrcA(D_icode, D_rA);
    d_srcB = getSrcB(D_icode, D_rB);

    uint64_t d_dstM = getDstM(D_icode, D_rA);
    uint64_t d_dstE = getDstE(D_icode, D_rB);

    uint64_t d_valA = selFwdA(d_srcA, pregs, stages);
    uint64_t d_valB = forwardB(d_srcB, pregs, stages);

    calculateControlSignals(pregs);

    setEInput(ereg, D_stat, D_icode, D_ifun, D_valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB);
    return false;
}
/* doClockHigh
 * applies the appropriate control signal to the D
 * and E register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
    E * ereg = (E *) pregs[EREG];
    if(E_bubble)
    {
         ereg->getstat()->bubble(SAOK);
         ereg->geticode()->bubble(INOP);
        ereg->getifun()->bubble();
        ereg->getvalC()->bubble();
        ereg->getvalA()->bubble();
        ereg->getvalB()->bubble();
        ereg->getdstE()->bubble(RNONE);
        ereg->getdstM()->bubble(RNONE);
        ereg->getsrcA()->bubble(RNONE);
        ereg->getsrcB()->bubble(RNONE);
    }
    else 
    {
        ereg->getstat()->normal();
        ereg->geticode()->normal();
        ereg->getifun()->normal();
        ereg->getvalC()->normal();
        ereg->getvalA()->normal();
        ereg->getvalB()->normal();
        ereg->getdstE()->normal();
        ereg->getdstM()->normal();
        ereg->getsrcA()->normal();
        ereg->getsrcB()->normal();

    }
}
/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: ereg - pointer to the E register instance
 * @param: stat - value to be stored in the stat pipeline register within E
 * @param: icode - value to be stored in the icode pipeline register within E
 * @param: ifun - value to be stored in the ifun pipeline register within E
 * @param: valC - value to be stored in the valC pipeline register within E
 * @param: valA - value to be stored in the valA pipeline register within E
 * @param: valB - value to be stored in the valB pipeline register within E
 * @param: dstE - value to be stored in the dstE pipeline register within E
 * @param: dstM - value to be stored in the dstM pipeline register within E
 * @param: srcA - value to be stored in the srcA pipeline register within E
 * @param: srcB - value to be stored in the srcB pipeline register within E
 */
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
        uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE,
        uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
    ereg->getstat()->setInput(stat);
    ereg->geticode()->setInput(icode);
    ereg->getifun()->setInput(ifun);
    ereg->getvalC()->setInput(valC);
    ereg->getvalA()->setInput(valA);
    ereg->getvalB()->setInput(valB);
    ereg->getdstE()->setInput(dstE);
    ereg->getdstM()->setInput(dstM);
    ereg->getsrcA()->setInput(srcA);
    ereg->getsrcB()->setInput(srcB);

}
uint64_t DecodeStage::getSrcA(uint64_t instr, uint64_t D_rA)
{
    if(instr == IOPQ || instr == IRMMOVQ || instr == IRRMOVQ || instr == IPUSHQ)
        return D_rA;
    if(instr == IPOPQ || instr == IRET)
        return RSP;
    return RNONE;
}
uint64_t DecodeStage::getSrcB(uint64_t instr, uint64_t D_rB)
{
    if(instr == IOPQ || instr == IRMMOVQ || instr == IMRMOVQ)
        return D_rB;
    if(instr == IPUSHQ || instr == IPOPQ || instr == ICALL || instr == IRET)
        return RSP;
    return RNONE;
}
uint64_t DecodeStage::getDstM(uint64_t instr, uint64_t D_rA)
{
    if(instr == IMRMOVQ || instr == IPOPQ)
        return D_rA;
    return RNONE;
}
uint64_t DecodeStage::getDstE(uint64_t instr, uint64_t D_rB)
{
    if(instr == IIRMOVQ || instr == IOPQ || instr == IRRMOVQ)
        return D_rB;
    if(instr == IPUSHQ || instr == IPOPQ || instr == ICALL || instr == IRET)
        return RSP;
    return RNONE;
}
uint64_t DecodeStage::selFwdA(uint64_t d_srcA, PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    uint64_t e_dstE = ((ExecuteStage *)stages[ESTAGE])->gete_dstE();
    uint64_t e_valE = ((ExecuteStage *)stages[ESTAGE])->gete_valE();

    uint64_t M_dstE = mreg->getdstE()->getOutput();
    uint64_t M_valE = mreg->getvalE()->getOutput();

    uint64_t M_dstM = mreg->getdstM()->getOutput();
    uint64_t M_valM = ((MemoryStage *) stages[MSTAGE])->getm_valM();

    uint64_t W_dstE = wreg->getdstE()->getOutput();
    uint64_t W_valE = wreg->getvalE()->getOutput();

    uint64_t W_dstM = wreg->getdstM()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();

    if (d_srcA == RNONE)
    {
        return 0;
    }
    else if(d_srcA == e_dstE)
    {
        return e_valE;
    }
    else if(d_srcA == M_dstM)
    {
        return M_valM;
    }
    else if(d_srcA == M_dstE)
    {
        return M_valE;
    }
    else if(d_srcA == W_dstM)
    {
        return W_valM;
    }
    else if(d_srcA == W_dstE)
    {
        return W_valE;
    }
    return reg->readRegister(d_srcA, regError);
}
uint64_t DecodeStage::forwardB(uint64_t d_srcB, PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];


    uint64_t e_dstE = ((ExecuteStage *) stages[ESTAGE])->gete_dstE();
    uint64_t e_valE = ((ExecuteStage *) stages[ESTAGE])->gete_valE();

    uint64_t M_dstE = mreg->getdstE()->getOutput();
    uint64_t M_valE = mreg->getvalE()->getOutput();

    uint64_t M_dstM = mreg->getdstM()->getOutput();
    uint64_t M_valM = ((MemoryStage *) stages[MSTAGE])->getm_valM();

    uint64_t W_dstE = wreg->getdstE()->getOutput();
    uint64_t W_valE = wreg->getvalE()->getOutput();

    uint64_t W_dstM = wreg->getdstM()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();

    if(d_srcB == RNONE)
    {
        return 0;
    }
    else if(d_srcB == e_dstE)
    {
        return e_valE;
    }
    else if(d_srcB == M_dstM)
    {
        return M_valM;
    }
    else if(d_srcB == M_dstE)
    {
        return M_valE;
    }
    else if(d_srcB == W_dstM)
    {
        return W_valM;
    }
    else if(d_srcB == W_dstE)
    {
        return W_valE;
    }
    return reg->readRegister(d_srcB, regError);
}

bool DecodeStage::gete_bubble(uint64_t E_icode, uint64_t E_dstM)
{
    return ( (E_icode == IMRMOVQ || E_icode == IPOPQ) && 
            (E_dstM == d_srcA || E_dstM == d_srcB) );
}

void DecodeStage::calculateControlSignals(PipeReg ** regs)
{
    E * ereg = (E*) regs[EREG];
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    E_bubble = gete_bubble(E_icode, E_dstM);
}
