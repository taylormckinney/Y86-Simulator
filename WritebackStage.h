//wb.h
class WritebackStage: public Stage
{
 private:
      RegisterFile * reg = RegisterFile::getInstance();
 public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
};
