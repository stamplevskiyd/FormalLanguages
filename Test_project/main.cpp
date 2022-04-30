#include <iostream>
#include <stack>

std::string poliz(const std::string& regex){

    // строит ПОЛИЗ, добавляет 0 в качестве eps

    std::stack<char> buffer;
    std::string processed_line;
    bool prev_terminal = false;  // попробуем расставить конкатенации таким способом
    for (char i : regex){
        if ('a' <= i and i<= 'z'){
            processed_line += i;
            if (prev_terminal)
                buffer.push('.');
            prev_terminal = true;
        }
        if (i == '(' or i == '*' or i == '|') {
            if (i == '|')  // eps. Пока только один случай
                processed_line += '0';
            buffer.push(i);
            prev_terminal = false;
        }
        if (i == ')') {
            prev_terminal = false;
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

int main(){
    std::string regex;
    std::cout << poliz("(|a)(bb)*") << std::endl;
    return 0;
}