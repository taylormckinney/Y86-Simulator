//wb.C
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
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg **pregs, Stage **stages) {

    W *wreg = (W *) pregs[WREG];
    uint64_t W_stat = wreg->getstat()->getOutput();
    if (W_stat != SAOK) {
        return true;
    }
    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg **pregs) {
    bool regError;
    W *wreg = (W *) pregs[WREG];
    uint64_t W_valE = wreg->getvalE()->getOutput();
    uint64_t W_dstE = wreg->getdstE()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();
    uint64_t W_dstM = wreg->getdstM()->getOutput();

    reg->writeRegister(W_valE, W_dstE, regError);
    reg->writeRegister(W_valM, W_dstM, regError);
}
