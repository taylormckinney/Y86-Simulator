//execute.h

class ExecuteStage: public Stage
{
   private:
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd, uint64_t valE, 
      uint64_t valA, uint64_t dstE, uint64_t dstM);
      uint64_t e_dstE, e_valE;
      bool M_bubble = false;
   
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t getaluA(uint64_t instr, uint64_t E_valA, uint64_t E_valC);
      uint64_t getaluB(uint64_t instr, uint64_t E_valB);
      uint64_t getaluFun(uint64_t instr, uint64_t E_ifun);
      bool set_cc(uint64_t instr,uint64_t m_stat, uint64_t W_stat);
      uint64_t gete_dstE(uint64_t instr, uint64_t e_Cnd, uint64_t E_dstE);
      uint64_t ALU(uint64_t a, uint64_t b, uint64_t fun);
      void CC(uint64_t a, uint64_t b, uint64_t aluResult, uint64_t ifun);
      uint64_t gete_dstE(){return e_dstE;};
      uint64_t gete_valE(){return e_valE;};
      uint64_t cond(uint64_t icode, uint64_t ifun);
      bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat);
};
