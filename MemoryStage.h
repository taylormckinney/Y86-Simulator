//mem.h
class MemoryStage: public Stage
{
   private:
      void setWInput(W* wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, 
      uint64_t dstE, uint64_t dstM);
      
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t memAddr(uint64_t instr, uint64_t M_valE, uint64_t M_valA);
      bool memRead(uint64_t instr);
      bool memWrite(uint64_t instr);
};
