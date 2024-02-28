#include "parser.h"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <stack>


Parser::Parser(const std::string & filename) : scan(filename), circuit(nullptr), counter(0) {}


Parser::~Parser() {}


void Parser::GetLex() {
    try {
        lex = scan.GetLex();
    } catch(int c) {
        if (c == -1) {
            lex = "End Of File";
        } else {
            throw std::invalid_argument("invalid character " + std::string(1, c));
        }
    }
}


void Parser::CHECK(const std::string & str) {
    if (lex != str) {
        throw std::invalid_argument("expected " + str + ", but found " + lex);
    }
    GetLex();
}


bool Parser::COMP(const std::string & str) {
    if (lex == str) {
        GetLex();
        return true;
    }
    return false;
}


size_t Parser::NUMBER() {
    size_t num;
    if (isdigit(lex[0])) {
        num = std::stoi(lex);
    } else {
        throw std::invalid_argument("expected number, but found " + lex);
    }
    GetLex();
    return num;
}


size_t Parser::CONSTANT_EXPRESSION() {
    size_t num;
    if (isdigit(lex[0])) {
        num = std::stoi(lex);
    } else if (parameters.find(lex) != parameters.end()) {
        num = parameters[lex];
    } else {
        throw std::invalid_argument("expected constant, but found " + lex);
    }
    GetLex();
    return num;
}


void Parser::IDENTIFIER() {
    if (!isalpha(lex[0]) && lex[0] != '_') {
        throw std::invalid_argument("expected identifier, but found " + lex);
    }
}


Circuit *Parser::Analyze() {
    circuit = new Circuit();
    GetLex();
    MODULE();
    //circuit->construct();
    return circuit;
}


void Parser::MODULE() { // MODULE -> "module" IDENTIFIER ("#" MODULE_PARAMETER_DECLARATION)? LIST_OF_PORTS ";" MODULE_ITEM
    CHECK("module");
    IDENTIFIER();
    circuit->setName(lex);
    GetLex();
    if (COMP("#")) {
        MODULE_PARAMETER_DECLARATION();
    }
    LIST_OF_PORTS();
    CHECK(";");
    MODULE_ITEM();
}


void Parser::MODULE_PARAMETER_DECLARATION() { // MODULE_PARAMETER_DECLARATION -> "(" LIST_OF_PARAMETERS ")"
    CHECK("(");
    LIST_OF_PARAMETERS();
    CHECK(")");
}


void Parser::LIST_OF_PARAMETERS() { // LIST_OF_PARAMETERS -> "parameter" PARAM_ASSIGNMENT ("," "parameter" PARAM_ASSIGNMENT)*
    do {
        CHECK("parameter");
        PARAM_ASSIGNMENT();
    } while (COMP(","));
}


void Parser::LIST_OF_PORTS() { // LIST_OF_PORTS -> "(" PORTS ")" | eps
    if (COMP("(")) {
        PORTS();
        CHECK(")");
    }
}


void Parser::PORTS() { // PORTS -> "input" (RANGE)? IDENTIFIER | "output" (RANGE)? IDENTIFIER | IDENTIFIER
    NetType mode = NET_DEFAULT;
    bool range = false;
    size_t begin, end;
    while (lex != ")" && lex != "input" && lex != "output") {
        GetLex();
    }
    if (lex == ")") {
        return;
    }
    do {
        if (COMP("input")) {
            mode = NET_INPUT;
            range = RANGE(begin, end);
        } else if (COMP("output")) {
            mode = NET_OUTPUT;
            range = RANGE(begin, end);
        }
        if (range) {
            for (size_t i = begin; i <= end; ++i) {
                circuit->addNet(lex + "[" + std::to_string(i) + "]", mode);
            }
        } else {
            circuit->addNet(lex, mode);
        }
        GetLex();
    } while (COMP(","));
}


bool Parser::RANGE(size_t & begin, size_t & end) { // RANGE -> "[" CONSTANT_EXPRESSION ":" CONSTANT_EXPRESSION "]"
    bool range = false;
    if (COMP("[")) {
        range = true;
        end = CONSTANT_EXPRESSION();
        CHECK(":");
        begin = CONSTANT_EXPRESSION();
        CHECK("]");
    }
    return range;
}


void Parser::MODULE_ITEM() { // MODULE_ITEM -> "endmodule" | DECLARATION ";" MODULE_ITEM
    if (lex != "endmodule") {
        DECLARATION();
        CHECK(";");
        MODULE_ITEM();
    }
}


void Parser::DECLARATION() { // DECLARATION -> "parameter" LIST_OF_PARAM_ASSIGNMENTS | "wire" LIST_OF_VARIABLES | "input" LIST_OF_VARIABLES | "output" LIST_OF_VARIABLES | "assign" ASSIGNMENT | GATE_DECLARATION
    if (COMP("parameter")) {
        LIST_OF_PARAM_ASSIGNMENTS();
    } else if (COMP("wire")) {
        LIST_OF_VARIABLES(NET_DEFAULT);
    } else if (COMP("input")) {
        LIST_OF_VARIABLES(NET_INPUT);
    } else if (COMP("output")) {
        LIST_OF_VARIABLES(NET_OUTPUT);
    } else if (COMP("assign")) {
        ASSIGNMENT();
    } else {
        GATE_DECLARATION();
    }
}


void Parser::LIST_OF_PARAM_ASSIGNMENTS() { // LIST_OF_PARAM_ASSIGNMENTS -> PARAM_ASSIGNMENT ("," PARAM_ASSIGNMENT)*
    do {
        PARAM_ASSIGNMENT();
    } while (COMP(","));
}


void Parser::PARAM_ASSIGNMENT() { // PARAM_ASSIGNMENT -> IDENTIFIER "=" CONSTANT_EXPRESSION
    IDENTIFIER();
    std::string param = lex;
    GetLex();
    CHECK("=");
    parameters[param] = CONSTANT_EXPRESSION();
}


void Parser::LIST_OF_VARIABLES(NetType type) { // LIST_OF_VARIABLES -> (RANGE)? IDENTIFIER ("," IDENTIFIER)*
    size_t begin, end;
    bool range = RANGE(begin, end);
    do {
        IDENTIFIER();
        if (range) {
            for (size_t i = begin; i <= end; ++i) {
                circuit->addNet(lex + "[" + std::to_string(i) + "]", type);
            }
        } else {
            circuit->addNet(lex, type);
        }
        GetLex();
    } while (COMP(","));
}


Function Parser::GetType(const std::string & op) {
    Function func = FUNCTION_VAR;
    if (op == "&&" || op == "&") {
        func = FUNCTION_AND;
    } else if (op == "||" || op == "|") {
        func = FUNCTION_OR;
    } else if (op == "^~" || op == "~^") {
        func = FUNCTION_XNOR;
    } else if (op == "^") {
        func = FUNCTION_XOR;
    } else if (op == "~") {
        func = FUNCTION_NOT;
    }
    return func;
}


void Parser::GATE_DECLARATION() { // GATE_DECLARATION -> GATETYPE GATE_INSTANCE | MODULE_INSTANTIATION
    Function func = GATETYPE();
    if (func != FUNCTION_VAR) {
        GetLex();
        Node *node = circuit->addNode(func);
        GATE_INSTANCE(node);
    } else {
        MODULE_INSTANTIATION();
    }
}


Function Parser::GATETYPE() { // GATETYPE -> "and" | "nand" | "or" | "nor" | "xor" | "not"
    Function func = FUNCTION_VAR;
    if (lex == "and") {
        func = FUNCTION_AND;
    } else if (lex == "nand") {
        func = FUNCTION_NAND;
    } else if (lex == "or") {
        func = FUNCTION_OR;
    } else if (lex == "nor") {
        func = FUNCTION_NOR;
    } else if (lex == "xor") {
        func = FUNCTION_XOR;
    } else if (lex == "xnor") {
        func = FUNCTION_XNOR;
    } else if (lex == "buf") {
        func = FUNCTION_BUF;
    } else if (lex == "cut") {
        func = FUNCTION_CUT;
    } else if (lex == "not") {
        func = FUNCTION_NOT;
    }
    return func;
}


void Parser::GATE_INSTANCE(Node *node) { // GATE_INSTANCE -> "(" IDENTIFIER TERMINALS_ITER ")"
    CHECK("(");
    IDENTIFIER();
    node->output_name = lex;
    GetLex();
    TERMINALS_ITER(node);
    CHECK(")");
}


void Parser::TERMINALS_ITER(Node *node) { // TERMINALS_ITER -> "," IDENTIFIER TERMINALS_ITER | eps
    if (COMP(",")) {
        if (lex == "0" || lex == "1") {
            node->input_names.push_back("constant_" + lex);
        } else {
            IDENTIFIER();
            node->input_names.push_back(lex);
        }
        GetLex();
        TERMINALS_ITER(node);
    }
}


void Parser::MODULE_INSTANTIATION() { // MODULE_INSTANTIATION -> IDENTIFIER MODULE_INSTANCE
    IDENTIFIER();
    Node *node = circuit->addNode(FUNCTION_CIRCUIT);
    node->output_name = lex + "_#" + std::to_string(counter++);
    //Parser module_parser(lex + ".v");
    GetLex();
    std::map<std::string, std::string> port_connections;    
    MODULE_INSTANCE(port_connections);
    std::map<std::string, std::string>::iterator it = port_connections.begin();
    for (; it != port_connections.end(); it++) {
        node->input_names.push_back(it->second);
    }
    //node->circuit = module_parser.Analyze();
}


void Parser::MODULE_INSTANCE(std::map<std::string, std::string> & port_connections) { // IDENTIFIER "(" LIST_OF_MODULE_CONNECTIONS ")" ("," --||--)*
    do {
        IDENTIFIER();
        GetLex();
        CHECK("(");
        LIST_OF_MODULE_CONNECTIONS(port_connections);
        CHECK(")");
    } while (COMP(","));
}


void Parser::LIST_OF_MODULE_CONNECTIONS(std::map<std::string, std::string> & port_connections) { // LIST_OF_MODULE_CONNECTIONS -> "." IDENTIFIER "(" IDENTIFIER ")" ("," --||--)* | eps
    if (lex == ")") {
        return;
    }
    do {
        CHECK(".");
        IDENTIFIER();
        std::string input_wire = lex;
        GetLex();
        CHECK("(");
        IDENTIFIER();
        port_connections[input_wire] = lex;
        GetLex();
        CHECK(")");
    } while (COMP(","));
}


void Parser::ASSIGNMENT() { // ASSIGNMENT -> IDENTIFIER "=" EXPRESSION
    IDENTIFIER();
    GetLex();
    CHECK("=");
    std::vector<std::string> buf;
    EXPRESSION(buf);
    std::stack<std::string> st;
    Function func;
    for (size_t i = 0; i < buf.size(); ++i) {
        func = GetType(buf[i]);
        if (func == FUNCTION_VAR) {
            st.push(buf[i]);
        } else {
            std::string netName = "net_#" + std::to_string(i);
            circuit->addNet(netName, NET_DEFAULT);
            Node *node = circuit->addNode(func);
            node->output_name = netName;
            if (func != FUNCTION_NOT) {
                node->input_names.push_back(st.top());
                st.pop();
            }
            node->input_names.push_back(st.top());
            st.pop();
            st.push(netName);
        }
    }
}


void Parser::EXPRESSION(std::vector<std::string> & buf) {  // A -> B "||" A | B
    B(buf);
    while (lex == "||") {
        GetLex();
        B(buf);
        buf.push_back("||");
    }
}


void Parser::B(std::vector<std::string> & buf) { // B -> C "&&" B | C
    C(buf);
    while (lex == "&&") {
        GetLex();
        C(buf);
        buf.push_back("&&");
    }
}


void Parser::C(std::vector<std::string> & buf) { // C -> D "|" C | D
    D(buf);
    while (lex == "|") {
        GetLex();
        D(buf);
        buf.push_back("|");
    }
}


void Parser::D(std::vector<std::string> & buf) { // D -> E "^" D | E "^~" D | E "~^" D | E
    E(buf);
    while (lex == "^" || lex == "^~" || lex == "~^") {
        std::string curr = lex;
        GetLex();
        E(buf);
        buf.push_back(curr);
    }
}


void Parser::E(std::vector<std::string> & buf) { // E -> F "&" E | F
    F(buf);
    while (lex == "&") {
        GetLex();
        F(buf);
        buf.push_back("&");
    }
}


void Parser::F(std::vector<std::string> & buf) { // F -> IDENTIFIER | "~" A | "(" A ")"
    if (COMP("(")) {
        EXPRESSION(buf);
        CHECK(")");
    } else if (COMP("~")) {
        if (COMP("(")) {
            EXPRESSION(buf);
            CHECK(")");
        } else {
            buf.push_back(lex);
            GetLex();
        }
        buf.push_back("~");
    } else {
        IDENTIFIER();
        buf.push_back(lex);
        GetLex();
    }
}
