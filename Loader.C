/**
 * Names: Taylor McKinney & Cori Ernhart
 * Team: 22
 */
#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <sstream>


#include "Loader.h"
#include "Memory.h"

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 
#define COLON 5 //location of the ':' character 

uint64_t prevAddr = 0;
uint64_t prevLength;

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
    int lineNumber = 1;
    if(!isValidFileName(argv[1]))
    {//if true, file is open; if false, failed to open file
        return;
    }
    inf.open(argv[1]);
    std::string line;
    while(std::getline(inf, line))
    {
        //std::cout << line << "\n"; //prints the file line by line
        if(hasErrors(line))
        {
            std::cout << "Error on line " << std::dec << lineNumber << ": " << line << std::endl;
            loaded = false;
            return;
        }
        loadLine(line);
        lineNumber++;
    }
    //If control reaches here then no error was found and the program
    //was loaded into memory.
    loaded = true;  
    inf.close();
}

/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
    return loaded;
}

bool Loader::isValidFileName(std::string fname) {
    if (fname.length() <= 3) 
    {
        return false;
    } 
    else if (fname.substr(fname.length() - 3, 3) != ".yo") 
    {
        return false;
    }
    inf.open(fname);
    if (!inf.is_open()) 
    {
        return false;
    }
    inf.close();
    return true;
}

void Loader::loadLine(std::string line)
{
    bool memError = false;
    if(line[0] == '0' && line[DATABEGIN] != ' ')
    {   
        int32_t addr = convertHex(line, ADDRBEGIN, ADDREND);
        int8_t byte = 0;
        Memory* mem = Memory::getInstance();
        for(int i=DATABEGIN; i < COMMENT; i++)
        {
            if(line[i] != ' ' && line[i+1] != ' ')
            {
                byte = convertHex(line, i, i+1);
                mem->putByte(byte, addr, memError); 
                i++;
                addr++; 
            }
        }
        prevAddr = addr;
    }
}

uint64_t Loader::convertHex(std::string line, int begin, int end) {
    uint64_t val;
    std::stringstream ss;
    ss << line.substr(begin, (end-begin)+1);
    ss >> std::hex >> val;
    return val;
}

/**
 * determines if the line contains any errors
 */
bool Loader::hasErrors(std::string line)
{
    int dataEnd = line.find(' ', DATABEGIN)-1; 
    if(((!isCommentLine(line) && (!validHex(line, DATABEGIN, dataEnd) 
                        || !validHex(line, ADDRBEGIN, ADDREND))) || line[COMMENT] != '|') 
            && line.length() >0 )
    {
        return true;
    }
    if(!isCommentLine(line) && (line[COLON] != ':' ||  line[COLON+1] != ' ')  && line.length() >0)
    {
        return true;
    }
    if(line[0]=='0' && line[1] !='x')
    {
        return true;
    }
    bool lastWasSpace = true;
    for(int i= DATABEGIN; i < dataEnd; i++)
    {
        if(!lastWasSpace && line[i] == ' ')
        {
            return true;
        }
    }
    for(int i=0; i< COMMENT; i++)
    {
        if(!validHex(line, i, i) && line[i] != ' ' && line[i] !=':' && line[i] != 'x')
        {
            return true;
        }
    }
    if(!isCommentLine(line) && ((dataEnd-DATABEGIN + 1) % 2 != 0 || (dataEnd-DATABEGIN) == 0))
    {
        return true;
    }
    uint64_t addr = convertHex(line, ADDRBEGIN, ADDREND);
    if(addr + (dataEnd-DATABEGIN) > (0xfff + 1) && !isCommentLine(line))
    {
        return true;
    }
    if(prevAddr > addr && !isCommentLine(line))
    {
        return true;
    }
    if (!isCommentLine(line) && validHex(line, DATABEGIN, dataEnd))
    {
        for (int i=dataEnd+1; i < COMMENT; i++)
        {
            if (line[i] != ' ')
            {
                return true;
            }
        }
}
    return false;
}

/**
 * returns true if given line is just a comment line
 *      -entirely blank until COMMENT
 *      -has address, then blank until COMMENT
 */
bool Loader::isCommentLine(std::string line)
{
    for(int i=(ADDREND +2); i < COMMENT; i++)
    {
        if(line[i] != ' ')
        {
            return false; 
        }
    }
    return true;
}

/**
 * determines if all values in a given range are hex digits 
 *  returns true if all values are valid hex
 */
bool Loader::validHex(std::string line, int start, int end)
{
    for(int i= start; i <=end; i++)
    {
        if(!isxdigit(line[i]) && line[i] != ' ')
        {
            return false;
        }
    }
    return true;
}
