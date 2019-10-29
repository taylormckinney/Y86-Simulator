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
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"

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
      
      uint64_t stat = mreg->getstat()->getOutput();
      uint64_t icode = mreg->geticode()->getOutput();
      uint64_t valE = mreg->getvalE()->getOutput();
      uint64_t valM = mreg->getvalM()->getOutput();
      uint64_t dstE = mreg->getdstE()->getOutput();
      uint64_t dstM = mreg->getdstM()->getOutput();
        
        
        setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
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
void MemoryStage::setEInput(W* wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, 
      uint64_t dstE, uint64_t dstM)
{
        wreg->getstat()->setInput(stat);
        wreg->geticode()->setInput(icode);
        wreg->getvalE()->setInput(valE);
        wreg->getvalM()->setInput(valM);
        wreg->getdstE()->setInput(dstE);
        wreg->getdstM()->setInput(dstM);
            
}
