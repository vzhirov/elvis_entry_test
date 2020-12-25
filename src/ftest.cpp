#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

const static std::string EXECUTABLE = "./elvis_vzhirov_bin";
const static std::string TEST_INPUT = "../tests/acer-xf290c.edid";
const static std::string TEST_NORM  = "../tests/acer-xf290c.edid.txt";

int main(int argc, char* argv[])
{
    std::string cmd = EXECUTABLE+" "+TEST_INPUT;
    std::string output;
    char ch;

    FILE* pipe = popen(cmd.c_str(),"r");
    if(!pipe){
        std::cerr << "[error] Cannot execute testable." << std::endl;
        return 3; }
    while((ch = fgetc(pipe))!= EOF)
        output += ch;
    fclose(pipe);
    output += "\n";

    std::fstream file(TEST_NORM.c_str());
    std::string line,norm;
    while(file.good()){
        std::getline(file, line);
        norm += line+"\n"; }
    
    if(norm=="\n"){
        std::cerr << "[error] Test norm file is empty or not found." << std::endl;
        return 1;
    }else if(output!=norm){
        std::cerr << "[error] Test failed. Output mismatched the norm" << std::endl
                  << "--- Result was (" << output.size()
                  << " bytes)----------------------------------------" << std::endl
                  << output << std::endl
                  << "--- Expected (" << norm.size()
                  << ")----------------------------------------" << std::endl
                  << norm << std::endl
                  << "--- End ----------------------------------------" << std::endl;
        return 2;
    }
    return 0;
}

// vim: ts=4 et

