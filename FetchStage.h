//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      uint64_t f_pc;
      bool memError;
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      bool F_stall, D_stall;

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t PCincrement(uint64_t f_pc, bool needsRegIds, bool needsValC);
      void getRegIds(Memory * mem, uint64_t& rA, uint64_t& rB);
      uint64_t getValC(Memory * mem, uint64_t f_icode);
      uint64_t getf_stat(uint64_t f_icode, bool memError);
      uint64_t getf_icode(bool mem_error, uint64_t mem_icode);
      uint64_t getf_ifun(bool mem_error, uint64_t mem_ifun);
      bool instrValid(uint64_t f_icode);
      bool getf_stall(uint64_t e_icode, uint64_t e_dstM, uint64_t d_srcA,
            uint64_t d_srcB);
      bool getd_stall(uint64_t e_icode, uint64_t e_dstM, uint64_t d_srcA,
                  uint64_t d_srcB);
      // bool getd_bubble(uint64_t e_icode);
     void calculateControlSignals(E * ereg, DecodeStage * dstage);
};
