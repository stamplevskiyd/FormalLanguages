#include "api.hpp"
#include <string>

enum Type {terminal, iteration, alternative, concatenation};

struct Node{
    Type type;
    unsigned int number;
    char value;
    Node *l, *r;  // потомки вершины
    bool nullable;
    std::set <unsigned int> firstpos;
    std::set <unsigned int> lastpos;
    Node();
    Node(unsigned int n, char v);
    Type get_type(char v);
    Node build_tree(const std::string &s);
};

Node::Node() {
    type = terminal;
    number = 0;
    l = nullptr;
    r = nullptr;
    firstpos = {};
    lastpos = {};
    nullable = false;
    value = ' ';
}

Node::Node(unsigned int n, char v) {
    number = n;
    value = v;
    type = get_type(v);
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

Node Node::build_tree(const std::string &s) {
    unsigned int current_number = 0;
    char symbol;
    Node *Root = new Node(current_number, ' ');
    switch (get_type(s[0])){
        case terminal:
            Root ->
    }
}

DFA re2dfa(const std::string &s) {
	DFA res = DFA(Alphabet(s));
	res.create_state("Start", true);
	res.set_initial("Start");
	return res;
}
