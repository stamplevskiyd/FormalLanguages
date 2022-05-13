#include <iostream>
#include <stack>
#include <set>
#include <cstring>
#include <vector>

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
    Node * build_tree(const std::string &s);
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
    std::cout << processed_regex << std::endl;

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


// Печать дерева из интернета. Только для самопроверки
void print_tree_structure(Node *bt, int spaces)
{
    if(bt != NULL)
    {
        print_tree_structure(bt->right, spaces + 5);
        for(int i = 0; i < spaces; i++)
            std::cout << ' ';
        std::cout << "   " << bt->value << std::endl;
        print_tree_structure(bt->left, spaces + 5);
    }
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


int main(){
    std::string regex, processed;
    std::cin >> regex;
    processed = to_poliz('(' + regex + ")#");  // для совпадения с семинарами
    std::cout << processed << std::endl;
    Node n;
    Node *root;
    root = n.build_tree(processed);
    std::vector<std::set<unsigned int>> table;
    table = followpos(root);
    std::cout << "--------" << std::endl;
    unsigned int i = 0;
    for (auto row: table){
        std::cout << '"' << i << "\" ";
        for (auto number: row)
            std::cout << number << ' ';
        std::cout << '\n';
        i++;
    }
    std::cout << "--------" << std::endl;
    print_tree_structure(root, 0);
    return 0;
}