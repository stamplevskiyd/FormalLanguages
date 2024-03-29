#include "api.hpp"
#include <string>
#include <iostream>
#include <stack>
#include <set>
#include <vector>
#include <map>

struct Node{
    unsigned int number;
    char value;
    Node *left, *right;  // потомки вершины
    bool nullable;
    std::set <unsigned int> firstpos;
    std::set <unsigned int> lastpos;
    Node();
    Node(unsigned int n, char v);
    static Node * build_tree(const std::string &s, Alphabet alpha);
    static Node * delete_tree(Node * root);
};

Node::Node() {
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
    left = nullptr;
    right = nullptr;
    nullable = true;
}


std::string to_poliz(const std::string& regex, Alphabet alpha){

    // строит ПОЛИЗ, добавляет 0 в качестве eps

    std::stack<char> buffer;
    std::string processed_line;
    std::string processed_regex;

    // расставляем знаки конкатенации
    for (unsigned int i = 0; i < regex.size(); i++){
        if (i == 0)
            processed_regex += regex[i];
        else{
            if (alpha.has_char(regex[i - 1])){
                if (alpha.has_char(regex[i]) or regex[i] == '('){
                    processed_regex += char(1230);
                    processed_regex += regex[i];
                }
                else
                    processed_regex += regex[i];
            }

            if (regex[i - 1] == '(') {
                if (regex[i] == '|' or regex[i] == ')')
                    processed_regex += char(1234);  // eps
                processed_regex += regex[i];
            }

            if (regex[i - 1] == '*') {
                if (regex[i] == '|' or regex[i] == ')')
                    processed_regex += regex[i];
                else{
                    processed_regex += char(1230);
                    processed_regex += regex[i];
                }
            }

            if (regex[i - 1] == '|') {
                if (regex[i] == ')' or regex[i] == '|' or regex[i] == '*')
                    processed_regex += char(1234);
                processed_regex += regex[i];
            }
            if (regex[i - 1] == ')') {
                if (regex[i] == '|' or regex[i] == ')' or regex[i] == '*')
                    processed_regex += regex[i];
                else {
                    processed_regex += char(1230);
                    processed_regex += regex[i];
                }
            }
        }
    }
    std::cout << processed_regex << std::endl;

    for (char symbol : processed_regex){
        if (alpha.has_char(symbol) or symbol == '#' or symbol == char(1234)){
            processed_line += symbol;
        }
        if (symbol == '(') {
            buffer.push(symbol);
        }
        if (symbol == '*'){
            processed_line += symbol;
        }
        if (symbol == char(1230)){
            buffer.push(symbol);
        }
        if (symbol == '|'){
            while (buffer.top() == char(1230)){
                processed_line += buffer.top();
                buffer.pop();
            }
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

Node * Node::build_tree(const std::string &s, Alphabet alpha) {
    std::stack<Node*> buffer;
    unsigned int current_number = 0;  // номер терминала, который сейчас будем обрабатывать
    Node *left, *right;
    Node *new_node;
    Node *root;

    for (char symbol: s){
        if (alpha.has_char(symbol) or symbol == char(1234) or symbol == '#'){
            new_node = new Node(current_number, symbol);
            if (symbol != char(1234)) {  // eps в first/lastpos не идет
                new_node->firstpos.insert(current_number);  // это - терминал
                new_node->lastpos.insert(current_number);
            }
            if (symbol == char(1234))
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

        if (symbol == char(1230)){
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

Node *Node::delete_tree(Node * root) {
    if (root->left)
        delete_tree(root->left);
    if (root->right)
        delete_tree(root->right);
    root->left = nullptr;
    root->right = nullptr;
    delete root;
    return nullptr;
}


void fill_table(Node *root, std::vector<std::set<unsigned int>> *table){

    // заполнить часть таблицы на основе одной вершины
    if (root->left)
        fill_table(root->left, table);
    if (root->right)
        fill_table(root->right, table);
    if (root->value == char(1230)){
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

std::string set_to_str(const std::set<unsigned int>& v){
    std::string line;
    for (auto num: v){
        line += std::to_string(num);
        line += ' ';
    }
    return line;
}

bool find_number_in_str(const std::string& s, unsigned int i){
    if (s.empty())
        return false;
    std::string number;
    std::set <unsigned int> arr;
    for (auto letter: s){
        if (letter == ' '){
            arr.insert(std::stoi(number));
            number.clear();
        }
        else
            number += letter;
    }
    return (arr.find(i) != arr.end());
}

DFA re2dfa(const std::string &s) {
    DFA res = DFA(Alphabet(s));
    std::string poliz = to_poliz('(' + s + ")#", res.get_alphabet());
    Node *root = Node::build_tree(poliz, res.get_alphabet());
    std::vector<std::set<unsigned int>> table;
    table = followpos(root);
    unsigned int last_num = table.size(); // номер символа конца последовательности
    std::vector<std::string> states; // набор уже записанных состояний

    // какой букве отвечают какие номера
    std::map<char, std::set<unsigned int>> relations;
    unsigned int number = 0;
    for (auto letter: poliz){  // заполнение таблицы соответствия букв номерам
        if (res.get_alphabet().has_char(letter)) {
            relations[letter].insert(number);
            number++;
        }
        if (letter == char(1234))
            number++;
    }

    // Создание начального состояния. Сразу проврим, является ли оно конечным
    if (root->firstpos.find(last_num) !=root->firstpos.end())
        res.create_state(set_to_str(root->firstpos), true);
    else
        res.create_state(set_to_str(root->firstpos), false);
    states.emplace_back(set_to_str(root->firstpos));  // запишем начальное состояние в список состояний
    Node::delete_tree(root);

    std::set<unsigned int> current_numbers;
    std::set<unsigned int> current_state; // в какое состояние нужно будет перейти
    bool found = false;
    unsigned int position_number = 0;
    bool changed = false; // поменялось ли что-нибудь за этот шаг

    while (true) {
        changed = false; // Добавление новых состояний
        for (auto letter: res.get_alphabet()) {
            current_numbers = relations[letter];  // какие номера соответствуют этой букве
            for (auto num: current_numbers) {
                if (find_number_in_str(states[position_number], num))  // если этот символ есть в followpos
                    // то добавляем его followpos в формируемое множество
                    current_state.insert(table[num].begin(), table[num].end());
            }
            if (!current_state.empty()) {
                for (int i = 0; i < states.size(); i++) {
                    if (states[i] == set_to_str(current_state)) {
                        // такое состояние есть, переход в него
                        res.set_trans(states[position_number], letter, states[i]);
                        found = true;
                        break;
                    }
                }
                if (!found) {  // такого состояния нет, создаем новое
                    states.emplace_back(set_to_str(current_state));
                    res.create_state(set_to_str(current_state), current_state.find(last_num) != current_state.end());
                    res.set_trans(states[position_number], letter, states.back());
                    changed = true;
                }
                current_state.clear();
            }
            found = false;
            // сформировали множество, в которое нужно перейти по этому символ
        }
        position_number++;
        if (!changed and position_number == states.size())
            break;
    }
    res.set_initial("0");
    return res;
}