#include <iostream>
#include <stack>

std::string poliz(std::string regex){
    std::stack<char> buffer;
    std::string processed_line;
    for (unsigned int i = 0; i < regex.size(); i++){
        if ('a' <= regex[i] <= 'z')
            processed_line += regex[i];
        if (regex[i] == '(' or regex[i] == '*' or regex[i] == '|')
            buffer.push(regex[i]);
        if (regex[i] = ')')
            while (buffer.top() != '('){
                processed_line += buffer.top();
                buffer.pop();
            }
    }
    return processed_line;
}

int main(){
    std::string regex;
    std::cout << poliz("a|b") << std::endl;
    return 0;
}