#include <iostream>
#include <stack>
#include <set>

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
    bool prev_terminal = false;  // попробуем расставить конкатенации таким способом
    for (char symbol : regex){
        if ('a' <= symbol and symbol <= 'z'){
            processed_line += symbol;
            if (prev_terminal)
                buffer.push('.');
            prev_terminal = true;
        }
        if (symbol == '(' or symbol == '*' or symbol == '|') {
            if (symbol == '|' and not prev_terminal)  // eps. Пока только один случай
                processed_line += '0';
            if (symbol == '(' and prev_terminal)
                buffer.push('.');
            buffer.push(symbol);
            prev_terminal = false;
        }
        if (symbol == ')') {
            prev_terminal = true;
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
    unsigned int current_number = 0;
    Node *left, *right;
    Node *new_node;
    Node *root = nullptr;



    for (char symbol: s){
        if (('a' <= symbol and symbol <= 'z') or symbol == '0'){
            new_node = new Node(current_number, symbol);
            new_node->firstpos.insert(current_number);  // это - терминал
            new_node->lastpos.insert(current_number);
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
    std::cout << current_number << std::endl;
    return root;
}


int main(){
    std::string regex, processed;
    std::cin >> regex;
    processed = to_poliz(regex);
    std::cout << processed << std::endl;
    Node n;
    n.build_tree(processed);
    return 0;
}