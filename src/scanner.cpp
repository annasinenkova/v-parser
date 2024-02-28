#include "scanner.h"
#include <iostream>


Scanner::Scanner(const std::string & filename) : c(' '), pos(0), buf(), fin(filename.c_str()) {
    std::string carr;
    while (getline(fin, carr)) {
        buf += carr;
    }
}


Scanner::~Scanner() {
    fin.close();
}


void Scanner::GetChar() {
    if (pos >= buf.size()) {
        c = -1;
    } else {
        c = buf[pos++];
    }
}


std::string Scanner::GetLex() {
    std::string lex;
    State CS = H;
    do {
        switch (CS) {
            case H:
                if (isspace(c)) {
                    GetChar();
                } else if (isdigit(c)) {
                    lex.push_back(c);
                    GetChar();
                    CS = NUM;
                } else if (isalpha(c) || c == '_') {
                    lex.push_back(c);
                    GetChar();
                    CS = IDENT;
                } else if (c == ',' || c == ';' || c == ':' || c == '(' || c == ')' || c == '=' || c == '[' || c == ']' || c == '#' || c == '.') {
                    lex.push_back(c);
                    GetChar();
                    return lex;
                } else if (c == '&' || c == '|' || c == '^' || c == '~') {
                    lex.push_back(c);
                    GetChar();
                    CS = DELIM;
                } else {
                    throw c;
                }
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c) || c == '_') {
                    lex.push_back(c);
                    GetChar();
                } else {
                    return lex;
                }
                break;
            case DELIM:
                if (c == '&' || c == '|' || c == '^' || c == '~') {
                    lex.push_back(c);
                    GetChar();
                } else {
                    return lex;
                }
                break;
            case NUM:
                if (c == '\'') {
                    GetChar();
                    if (c != 'b') {
                        throw c;
                    }
                    GetChar();
                    if (isdigit(c)) {
                        lex = std::string(1, c);
                        GetChar();
                        return lex;
                    }
                    throw c;
                } else if (isdigit(c)) {
                    lex.push_back(c);
                    GetChar();
                } else {
                    return lex;
                }
                break;
            default:
                throw c;
        }
    } while(true);
}

