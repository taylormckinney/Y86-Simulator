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

/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
        D * dreg = (D *) pregs[DREG];
        E * ereg = (E *) pregs[EREG];
        
        
        
        setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
        return false;
}
/* doClockHigh
 * applies the appropriate control signal to the D
 * and E register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
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
void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
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
