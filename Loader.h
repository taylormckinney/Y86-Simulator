
class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle 
   public:
      Loader(int argc, char * argv[]);
      bool isLoaded();
      bool iSValidFileName(string fname);
      void loadLine(string line);
      uint64_t convertHex(string line, int begin, int end);

};
