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
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"

/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
      E * ereg = (E*) pregs[EREG];
      M * mreg = (M*) pregs[MREG];
  
      uint64_t stat = ereg->getstat()->getOutput();
      uint64_t icode = ereg->geticode()->getOutput();
      uint64_t Cnd = 0;
      uint64_t valE = 0;
      uint64_t valA = ereg->getvalA()->getOutput();
      uint64_t dstE = ereg->getdstE()->getOutput();
      uint64_t dstM = ereg->getdstM()->getOutput();
        
        setMInput(mreg, stat, icode, Cnd, valE, valA, dstE, dstM);
        return false;
}
/* doClockHigh
 * applies the appropriate control signal to the register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
        M * mreg = (M*) pregs[MREG];
        mreg->getstat()->normal();
        mreg->geticode()->normal();
        mreg->getCnd()->normal();
        mreg->getvalE()->normal();
        mreg->getvalA()->normal();
        mreg->getdstE()->normal();
        mreg->getdstM()->normal();
}
/* setMInput
 * provides the input to potentially be stored in the M register
 * during doClockHigh
 *

*/
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd, uint64_t valE, 
      uint64_t valA, uint64_t dstE, uint64_t dstM)
{
        mreg->getstat()->setInput(stat);
        mreg->geticode()->setInput(icode);
        mreg->getCnd()->setInput(Cnd);
        mreg->getvalE()->setInput(valE);
        mreg->getvalA()->setInput(valA);
        mreg->getdstE()->setInput(dstE);
        mreg->getdstM()->setInput(dstM);
            
}