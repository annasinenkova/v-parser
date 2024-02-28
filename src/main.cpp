#include "parser.h"
#include "circuit.h"
#include <iostream>
#include <exception>
#include <stdexcept>


int main(int agrc, char *argv[]) {
    if (agrc <= 1) {
        std::cout << "Error: no file name" << std::endl;
        return 0;
    }
    try {
        Parser p(argv[1]);
        Circuit *circuit = p.Analyze();
        circuit->print();
    } catch(const std::invalid_argument & err) {
        std::cout << "Error: " << err.what() << std::endl;
    }
    return 0;
}
