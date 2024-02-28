#pragma once

#include "scanner.h"
#include "circuit.h"
#include <vector>
#include <map>

class Parser {
private:
    Scanner scan;
    Circuit *circuit;
    size_t counter;
    std::string lex;
    std::map<std::string, size_t> parameters;

public:
    Parser(const std::string &);
    ~Parser();
    Circuit *Analyze();

private:
    void GetLex();

    void MODULE();
    void MODULE_PARAMETER_DECLARATION();
    void LIST_OF_PARAMETERS();
    void PARAM_ASSIGNMENT();
    void LIST_OF_PORTS();
    void PORTS();
    void MODULE_ITEM();
    void DECLARATION();
    void LIST_OF_PARAM_ASSIGNMENTS();
    void LIST_OF_VARIABLES(NetType);
    void GATE_DECLARATION();
    void GATE_INSTANCE(Node *);
    void TERMINALS_ITER(Node *);
    void MODULE_INSTANTIATION();
    void MODULE_INSTANCE(std::map<std::string, std::string> &);
    void LIST_OF_MODULE_CONNECTIONS(std::map<std::string, std::string> &);
    void ASSIGNMENT();
    void EXPRESSION(std::vector<std::string> &);
    void B(std::vector<std::string> &);
    void C(std::vector<std::string> &);
    void D(std::vector<std::string> &);
    void E(std::vector<std::string> &);
    void F(std::vector<std::string> &);

    size_t NUMBER();
    size_t CONSTANT_EXPRESSION();
    void IDENTIFIER();
    void CHECK(const std::string &);
    bool COMP(const std::string &);
    bool RANGE(size_t & begin, size_t & end);

    Function GATETYPE();
    Function GetType(const std::string &);
};
