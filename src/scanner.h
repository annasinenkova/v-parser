#pragma once

#include <cstdio>
#include <fstream>


class Scanner {
private:
    enum State { H, IDENT, DELIM, NUM };
    
    int c;
    size_t pos;
    std::string buf;
    std::ifstream fin;
    
public:
    Scanner(const std::string &);
    ~Scanner();
    std::string GetLex();

private:  
    void GetChar();
};

