#include "api.hpp"
#include <string>
#include <iostream>
#include <stack>
#include <set>
#include <cstring>
#include <vector>
#include <map>

enum Type {terminal, iteration, alternative, concatenation};

struct Node{
    Type type;
    unsigned int number;
    char value;
    Node *left, *right;  // потомки вершины
    bool nullable;
    std::set <unsigned int> firstpos;
    std::set <unsigned int> lastpos;
    Node();
    Node(unsigned int n, char v);
    Type get_type(char v);
    static Node * build_tree(const std::string &s);
};

Node::Node() {
    type = terminal;
    number = 0;
    left = nullptr;
    right = nullptr;
    firstpos = {};
    lastpos = {};
    nullable = false;
    value = ' ';
}

Node::Node(unsigned int n, char v) {
    number = n;
    value = v;
    type = get_type(v);
    left = nullptr;
    right = nullptr;
}

Type Node::get_type(char v) {
    if (v == '*')
        return iteration;
    if (v == '|')
        return alternative;
    if (v == '.')
        return concatenation;
    return terminal;  // пока непонятно, что делать с конкатенацией
}

std::string to_poliz(const std::string& regex){

    // строит ПОЛИЗ, добавляет 0 в качестве eps

    std::stack<char> buffer;
    std::string processed_line;
    std::string processed_regex;

    // расставляем знаки конкатенации
    for (unsigned int i = 0; i < regex.size(); i++){
        if (i == 0)
            processed_regex += regex[i];
        else{
            if ('a' <= regex[i - 1] and regex[i - 1] <= 'z'){
                if (('a' <= regex[i] and regex[i] <= 'z') or regex[i] == '('){
                    processed_regex += '.';
                    processed_regex += regex[i];
                }
                else
                    processed_regex += regex[i];
            }

            if (regex[i - 1] == '(') {
                if (regex[i] == '|')
                    processed_regex += '0';  // eps
                processed_regex += regex[i];
            }

            if (regex[i - 1] == '*') {
                if (regex[i] == '|' or regex[i] == ')')
                    processed_regex += regex[i];
                else{
                    processed_regex += '.';
                    processed_regex += regex[i];
                }
            }

            if (regex[i - 1] == '|') {
                processed_regex += regex[i];
            }
            if (regex[i - 1] == ')') {
                if (regex[i] == '|' or regex[i] == ')' or regex[i] == '*')
                    processed_regex += regex[i];
                else {
                    processed_regex += '.';
                    processed_regex += regex[i];
                }
            }
        }
    }

    for (char symbol : processed_regex){
        if (('a' <= symbol and symbol <= 'z') or symbol == '#' or symbol == '0' or symbol == '*'){
            processed_line += symbol;
        }
        if (symbol == '('  or symbol == '|' or symbol == '.') {
            buffer.push(symbol);
        }
        if (symbol == ')') {
            while (buffer.top() != '(') {
                processed_line += buffer.top();
                buffer.pop();
            }
            buffer.pop(); // убрать открывающую скобку
        }
    }
    while(!buffer.empty()){
        processed_line += buffer.top();
        buffer.pop();
    }
    return processed_line;
}

Node * Node::build_tree(const std::string &s) {
    std::stack<Node*> buffer;
    unsigned int current_number = 0;  // номер терминала, который сейчас будем обрабатывать
    Node *left, *right;
    Node *new_node;
    Node *root = nullptr;

    for (char symbol: s){
        if (('a' <= symbol and symbol <= 'z') or symbol == '0' or symbol == '#'){
            new_node = new Node(current_number, symbol);
            if (symbol != '0') {  // eps в first/lastpos не идет
                new_node->firstpos.insert(current_number);  // это - терминал
                new_node->lastpos.insert(current_number);
            }
            if (symbol == '0')
                new_node->nullable = true;
            else
                new_node->nullable = false;
            buffer.push(new_node);
            current_number++;
        }

        if (symbol == '*'){
            left = buffer.top();  // пусть в этом случае потомок будет слева
            buffer.pop();
            new_node = new Node(0, symbol);
            new_node->left = left;
            new_node->nullable = true;
            new_node->firstpos = left->firstpos;
            new_node->lastpos = left->lastpos;

            buffer.push(new_node);
        }

        if (symbol == '|'){
            right = buffer.top();
            buffer.pop();
            left = buffer.top();
            buffer.pop();
            new_node = new Node(0, symbol);
            new_node->right = right;
            new_node->left = left;

            new_node->firstpos.insert(left->firstpos.begin(), left->firstpos.end());  // объединение списков
            new_node->firstpos.insert(right->firstpos.begin(), right->firstpos.end());

            new_node->lastpos.insert(left->lastpos.begin(), left->lastpos.end());
            new_node->lastpos.insert(right->lastpos.begin(), right->lastpos.end());

            if (left->nullable or right->nullable)
                new_node->nullable = true;
            else
                new_node->nullable = false;

            buffer.push(new_node);
        }

        if (symbol == '.'){
            right = buffer.top();
            buffer.pop();
            left = buffer.top();
            buffer.pop();
            new_node = new Node(0, symbol);
            new_node->right = right;
            new_node->left = left;

            new_node->firstpos.insert(left->firstpos.begin(), left->firstpos.end());  // левое есть всегда,
            // но если оно обнуляется - то и правое
            if (left->nullable)
                new_node->firstpos.insert(right->firstpos.begin(), right->firstpos.end());

            new_node->lastpos.insert(right->lastpos.begin(), right->lastpos.end());  // здесь наоборот
            if (right->nullable)
                new_node->lastpos.insert(left->lastpos.begin(), left->lastpos.end());

            if (left->nullable and right->nullable)
                new_node->nullable = true;
            else
                new_node->nullable = false;

            buffer.push(new_node);
        }
    }
    root = buffer.top();
    return root;
}


void fill_table(Node *root, std::vector<std::set<unsigned int>> *table){

    // заполнить часть таблицы на основе одной вершины
    if (root->left)
        fill_table(root->left, table);
    if (root->right)
        fill_table(root->right, table);
    if (root->value == '.'){
        for (auto left_last: root->left->lastpos) {
            (*table)[left_last].insert(root->right->firstpos.begin(), root->right->firstpos.end());
        }
    }
    if (root->value == '*'){
        for (auto left_last: root->left->lastpos) {
            (*table)[left_last].insert(root->left->firstpos.begin(), root->left->firstpos.end());
        }
    }
}

std::vector<std::set<unsigned int>> followpos(Node *root){
    // считает followpos для заданного дерева и строит табличку
    std::vector<std::set<unsigned int>> followpos_table;
    unsigned int number;
    number = root->right->number;  // там #, а это всегда последний номер
    for (unsigned int i = 0; i < number; i++)
        followpos_table.emplace_back();  // записать в конец пустое множество
    fill_table(root, &followpos_table);
    return followpos_table;
}

struct State{
    std::set<unsigned int> symbols;
    bool processed;
    State();
    State(std::set <unsigned int>symbols);
    bool operator== (const State& new_state);
};

State::State() {
    symbols = {};
    processed = false;
}

State::State(std::set <unsigned int> syms) {
    symbols = syms;
    processed = false;
}

bool State::operator== (const State& new_state) {  // равенство состояний - по набору символов
    if (this->symbols == new_state.symbols)
        return true;
    return false;
}


DFA re2dfa(const std::string &s) {
    std::string poliz = to_poliz('(' + s + ")#");
    Node *root = Node::build_tree(poliz);
    std::vector<std::set<unsigned int>> table;
    table = followpos(root);
	DFA res = DFA(Alphabet(s));
    unsigned int last_num = table.size(); // номер символа конца последовательности
    std::vector<State> states; // набор уже записанных состояний

    // какой букве отвечают какие номера
    std::map<char, std::set<unsigned int>> relations;
    unsigned int number = 0;
    for (auto letter: poliz){  // заполнение таблицы соответствия букв номерам
        if ('a' <= letter and letter <= 'z') {
            relations[letter].insert(number);
            number++;
        }
        if (letter == '0')
            number++;
    }

    // Создание начального состояния. Сразу проврим, является ли оно конечным
    if (root->firstpos.find(last_num) !=root->firstpos.end())
        res.create_state("0", true);
    else
        res.create_state("0", false);
    states.emplace_back( root->firstpos);  // запишем начальное состояние в список состояний

    std::set<unsigned int> current_numbers;
    std::set<unsigned int> current_state; // в какое состояние нужно будет перейти
    bool found = false;
    unsigned int position_number = 0;

    while (!states.back().processed) {
        for (auto letter: res.get_alphabet()) {
            current_numbers = relations[letter];  // какие номера соответствуют этой букве
            for (auto num: current_numbers) {
                if (states[position_number].symbols.find(num) != states[position_number].symbols.end())  // если этот символ есть в followpos
                    // то добавляем его followpos в формируемое множество
                    current_state.insert(table[num].begin(), table[num].end());
            }
            if (!current_state.empty()) {
                for (int i = 0; i < states.size(); i++) {
                    if (states[i] == State(current_state)) {
                        // такое состояние есть, переход в него
                        res.set_trans(std::to_string(position_number), letter, std::to_string(i));
                        found = true;
                        break;
                    }
                }
                if (!found) {  // такого состояния нет, создаем новое
                    states.emplace_back(current_state);
                    res.create_state(std::to_string(states.size() - 1), current_state.find(last_num) != current_state.end());
                    res.set_trans(std::to_string(position_number), letter, std::to_string(states.size() - 1));
                }
                current_state.clear();
            }
            // сформировали множество, в которое нужно перейти по этому символ
        }
        states[position_number].processed = true;
        position_number++;
    }
	return res;
}
