#include "parsingFunctions.h"
#include "stack.hpp"
#include "queue.hpp"

void asafov::str_to_queue(queue<std::string>& queue, const std::string& str) {
    size_t checkpoint = 0;
    for (size_t pos = 0; pos < str.size(); pos++) {
        if (str[pos] == '(' || str[pos] == ')' || str[pos] == '+' || str[pos] == '-' ||
            str[pos] == '*' || str[pos] == '/' || str[pos] == '%') {
            if (checkpoint != pos) {
                queue.push(str.substr(checkpoint, pos - checkpoint));
            }
            queue.push(str.substr(pos, 1));
            checkpoint = pos + 1;
            }
        else if (isdigit(str[pos])) {
            continue;
        }
        else {
            throw std::invalid_argument("invalid char!");
        }
    }
    
    if (checkpoint < str.size()) {
        queue.push(str.substr(checkpoint));
    }
}

void asafov::into_polish(asafov::queue<std::string>& q)
{
}

//double asafov::count(asafov::queue<std::string>& queue)
//{
    
//}