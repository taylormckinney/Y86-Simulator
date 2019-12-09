#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "ConditionCodes.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Tools.h"

/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg **pregs, Stage **stages) {
    E *ereg = (E *) pregs[EREG];
    M *mreg = (M *) pregs[MREG];
    W *wreg = (W *) pregs[WREG];

    uint64_t E_stat = ereg->getstat()->getOutput();
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t E_ifun = ereg->getifun()->getOutput();
    uint64_t E_valC = ereg->getvalC()->getOutput(); //set e_valE=E_valC
    uint64_t E_valA = ereg->getvalA()->getOutput();
    uint64_t E_valB = ereg->getvalB()->getOutput();
    uint64_t E_dstE = ereg->getdstE()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();

    uint64_t m_stat = ((MemoryStage *) stages[MSTAGE])->getm_stat();
    uint64_t W_stat = wreg->getstat()->getOutput();

    //get ALU inputs
    uint64_t a = getaluA(E_icode, E_valA, E_valC);
    uint64_t b = getaluB(E_icode, E_valB);

    //actually send values to ALU:
    e_valE = ALU(a, b, getaluFun(E_icode, E_ifun));

    //set CC if necessary:
    if (set_cc(E_icode, m_stat, W_stat)) {
        CC(a, b, e_valE, E_ifun);
    }

    M_bubble = calculateControlSignals(m_stat, W_stat);

    e_Cnd = cond(E_icode, E_ifun);

    e_dstE = gete_dstE(E_icode, e_Cnd, E_dstE);

    //updated M reg
    setMInput(mreg, E_stat, E_icode, e_Cnd, e_valE, E_valA, e_dstE, E_dstM);

    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg **pregs) {
    M *mreg = (M *) pregs[MREG];
    if (M_bubble) {
        mreg->getstat()->bubble(SAOK);
        mreg->geticode()->bubble(INOP);
        mreg->getCnd()->bubble();
        mreg->getvalE()->bubble();
        mreg->getvalA()->bubble();
        mreg->getdstE()->bubble(RNONE);
        mreg->getdstM()->bubble(RNONE);
    } else {
        mreg->getstat()->normal();
        mreg->geticode()->normal();
        mreg->getCnd()->normal();
        mreg->getvalE()->normal();
        mreg->getvalA()->normal();
        mreg->getdstE()->normal();
        mreg->getdstM()->normal();
    }
}

/* setMInput
 * provides the input to potentially be stored in the M register
 * during doClockHigh
 *

*/
void ExecuteStage::setMInput(M *mreg, uint64_t stat, uint64_t icode, uint64_t Cnd, uint64_t valE,
                             uint64_t valA, uint64_t dstE, uint64_t dstM) {
    mreg->getstat()->setInput(stat);
    mreg->geticode()->setInput(icode);
    mreg->getCnd()->setInput(Cnd);
    mreg->getvalE()->setInput(valE);
    mreg->getvalA()->setInput(valA);
    mreg->getdstE()->setInput(dstE);
    mreg->getdstM()->setInput(dstM);

}

uint64_t ExecuteStage::getaluA(uint64_t instr, uint64_t E_valA, uint64_t E_valC) {
    if (instr == IRRMOVQ || instr == IOPQ) {
        return E_valA;
    }
    if (instr == IIRMOVQ || instr == IRMMOVQ || instr == IMRMOVQ) {
        return E_valC;
    }
    if (instr == ICALL || instr == IPUSHQ) {
        return -8;
    }
    if (instr == IRET || instr == IPOPQ) {
        return 8;
    }
    return 0;
}

uint64_t ExecuteStage::getaluB(uint64_t instr, uint64_t E_valB) {
    if (instr == IRMMOVQ || instr == IMRMOVQ || instr == IOPQ || instr == ICALL
        || instr == IPUSHQ || instr == IRET || instr == IPOPQ) {
        return E_valB;
    }
    return 0;
}

uint64_t ExecuteStage::getaluFun(uint64_t instr, uint64_t E_ifun) {
    if (instr == IOPQ) {
        return E_ifun;
    }
    return ADDQ;
}

bool ExecuteStage::set_cc(uint64_t instr, uint64_t m_stat, uint64_t W_stat) {
    return (instr == IOPQ && (m_stat == SAOK) && (W_stat == SAOK));
}

uint64_t ExecuteStage::gete_dstE(uint64_t instr, uint64_t e_Cnd, uint64_t E_dstE) {
    if (instr == IRRMOVQ && !e_Cnd) {
        return RNONE;
    }
    return E_dstE;
}

uint64_t ExecuteStage::ALU(uint64_t a, uint64_t b, uint64_t fun) {
    if (fun == ADDQ) {
        return b + a;
    }
    if (fun == SUBQ) {
        return b - a;
    }
    if (fun == ANDQ) {
        return b & a;
    }
    if (fun == XORQ) {
        return b ^ a;
    }
    return 0;
}

void ExecuteStage::CC(uint64_t a, uint64_t b, uint64_t aluResult, uint64_t ifun) {
    bool ccError;
    ConditionCodes *cc = ConditionCodes::getInstance();
    if (aluResult == 0) {
        cc->setConditionCode(1, ZF, ccError);
    } else {
        cc->setConditionCode(0, ZF, ccError);
    }

    cc->setConditionCode(Tools::sign(aluResult), SF, ccError);
    if (ifun == ADDQ && Tools::addOverflow(a, b)) {
        cc->setConditionCode(1, OF, ccError);
    } else if (ifun == SUBQ && Tools::subOverflow(a, b)) {
        cc->setConditionCode(1, OF, ccError);
    } else {
        cc->setConditionCode(0, OF, ccError);
    }
}

uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun) {
    bool ccError;
    ConditionCodes *cc = ConditionCodes::getInstance();
    uint64_t zf = cc->getConditionCode(ZF, ccError);
    uint64_t sf = cc->getConditionCode(SF, ccError);
    uint64_t of = cc->getConditionCode(OF, ccError);

    if (icode != IJXX && icode != ICMOVXX) {
        return 0;
    } else if (ifun == UNCOND) {
        return 1;
    } else if (ifun == LESSEQ) {
        return (sf ^ of) | zf;
    } else if (ifun == LESS) {
        return (sf ^ of);
    } else if (ifun == EQUAL) {
        return zf;
    } else if (ifun == NOTEQUAL) {
        return !zf;
    } else if (ifun == GREATEREQ) {
        return !(sf ^ of);
    } else if (ifun == GREATER) {
        return !(sf ^ of) & !zf;
    }
    return 0;
}

bool ExecuteStage::calculateControlSignals(uint64_t m_stat, uint64_t W_stat) {
    return (m_stat != SAOK || W_stat != SAOK);
}
