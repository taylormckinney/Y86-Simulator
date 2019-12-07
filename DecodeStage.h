//class to perform the combinational logic of
////the Decode stage
class DecodeStage: public Stage
{
    private:
        void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
                uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, 
                uint64_t dstM, uint64_t srcA, uint64_t srcB); 

        RegisterFile * reg = RegisterFile::getInstance();
        bool regError;
        uint64_t d_srcA, d_srcB;
        bool E_bubble;

    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
        uint64_t getSrcA(uint64_t instr, uint64_t D_rA);
        uint64_t getSrcB(uint64_t instr, uint64_t D_rB);
        uint64_t getDstM(uint64_t instr, uint64_t D_rA);
        uint64_t getDstE(uint64_t instr, uint64_t D_rB);
        uint64_t selFwdA(uint64_t d_srcA, PipeReg ** pregs, Stage ** stages);
        uint64_t forwardB(uint64_t d_srcB, PipeReg ** pregs, Stage ** stages);
        uint64_t getd_srcA(){return d_srcA;};
        uint64_t getd_srcB(){return d_srcB;};
        void calculateControlSignals(PipeReg ** pregs);
        // for jxx implementation
        bool gete_bubble(uint64_t e_icode, uint64_t e_Cnd, uint64_t e_dstM);
};
