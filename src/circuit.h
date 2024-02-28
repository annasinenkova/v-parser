#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>

#define CONSTANT_0 "%item_constant_0%"
#define CONSTANT_1 "%item_constant_1%"

/// Тип узла
enum NodeType {
    NODE_DEFAULT, ///< Обычный функциональный элемент
    NODE_INPUT, ///< Узел-вход. Создаётся автоматически при построении схемы
    NODE_CONSTANT, ///< Узел-константа.
};

/// Логическая функция
enum Function {
    FUNCTION_AND, 
    FUNCTION_NAND,
    FUNCTION_OR,
    FUNCTION_NOR,
    FUNCTION_XOR,
    FUNCTION_XNOR,
    FUNCTION_BUF,
    FUNCTION_NOT,
    FUNCTION_CUT,
    FUNCTION_VAR,
    FUNCTION_CIRCUIT
};

/// Узел
struct Node {
    std::string name; ///< Имя узла
    NodeType type; ///< Тип узла
    union {
        Function function; ///< Функция, реализуемая в узле
        bool value; ///< Константа узла-констаны
    };
    mutable bool lazy, lazy_value; ///< Вспомогательные переменные для ленивых вычислений значения функции в узле
    std::vector<Node *> input; ///< Указатели на входные узлы
    std::vector<Node *> output; ///< Указатели на выходные узлы
    std::vector<std::string> input_names; ///< Имена нетов входов
    std::string output_name; ///< Имя нета выхода
    Node *original; ///< Узел исходной схемы, соответствующий узлу конуса
    Node(NodeType _type, Function _function); 
    Node(NodeType _type, bool _value);
    bool eval() const; ///< Рекурсивно вычисляет значение на выходе узла
};

/// Тип нета
enum NetType {
    NET_DEFAULT, ///< Обычный нет
    NET_INPUT, ///< Вход
    NET_OUTPUT, ///< Выход
    NET_CONSTANT ///< Константа
};

/// Нет
struct Net {
    std::string name; ///< Имя нета
    NetType type; ///< Тип нета
    Node *input; ///< Узел, выход которого соединён с данным нетом. Определяется автоматические при построении схемы.
    Net();
    Net(const std::string &_name, NetType _type);
    void setInput(Node *node); 
};

/// Схема
class Circuit {
    std::string name;
    static Node node_constant_0; ///< Узел-константа 0
    static Node node_constant_1; ///< Узел-константа 1
    std::vector<Node *> nodes; ///< Список всех обычных узлов схемы
    std::vector<Node *> service_nodes; ///< Список вспомогательных узлов схемы
    std::map<std::string, Net> nets; ///< Множество нетов схемы
    std::vector<std::string> inputs; ///< Имена всех нетов, являющихся входами
    std::vector<std::string> outputs; ///< Имена всех нетов, являющихся выходами
    std::map<std::string, std::string> renames;
    Circuit(const Circuit &);
    Circuit &operator=(const Circuit &);
    void setNetInput(const std::string &name, Node *node); ///< Привязка выхода узла к нету
    Node *addNode(NodeType type, Function function); 
    Node *addNode(NodeType type, bool value);
    void topsort(Node *node, std::set<Node *> &used,
        std::vector<Node *> &result) const; ///< Топологическая сортировка узлов
    const std::string &wire_name(const std::string &name) const;
public:
    Circuit();
    ~Circuit();
    void setName(const std::string &new_name);
    Node *addNode(Function function); ///< Добавление узла типа NODE_DEFAULT
    void addNet(const std::string &name, NetType type); ///< Добавление нета
    void construct(); ///< Построение схемы. Вызывается после добавления всех нетов, узлов схемы, а также заполнения input_names и output_name для всех этих узлов. 
    void print(bool abc_valid = false) const; ///< Вывод схемы в формате Verilog на стандартный поток вывода
    const std::vector<std::string> &getInputs() const; ///< Получение списка имён нетов, являющихся входами
    const std::vector<std::string> &getOutputs() const; ///< Получение списка имён нетов, являющихся выходами
    Node *getNetInput(const std::string &name) const; ///< Получение узла, выход которого связан с данным нетом
    void setInputValue(const std::string &name, bool value); ///< Установка значения на вход схемы
    bool getInputValue(const std::string &name) const; ///< Получение значения на входе схемы
    bool evalOutput(const std::string &name) const; ///< Вычисление значения на выходе схемы 
    const std::vector<Node *> &getNodes() const; ///< Получение списка узлов
    const std::map<std::string, Net> &getNets() const; ///< Получение нетов
    NetType getNetType(const std::string &name) const; ///< Получение типа нета
    void sortNodes(); ///< Топологическая сортировка узлов схемы
    void renameNet(const std::string &old_name, const std::string &new_name); ///< Переименование нета для вывода
    void clearRenames(); ///< Очистка переименований
};
