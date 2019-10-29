//execute.h

class ExecuteStage: public Stage
{
   private:
      void setMInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t Cnd, uint64_t valE, 
      uint64_t valA, uint64_t dstE, uint64_t dstM);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
};
