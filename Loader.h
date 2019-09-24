
class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle 
   public:
      Loader(int argc, char * argv[]);
      bool isLoaded();
      bool isValidFileName(std::string fname);
      void loadLine(std::string line);
      uint64_t convertHex(std::string line, int begin, int end);
    
};
