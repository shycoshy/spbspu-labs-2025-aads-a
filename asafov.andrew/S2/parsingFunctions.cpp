#include "parsingFunctions.h"
#include "stack.hpp"
#include "queue.hpp"

void asafov::str_to_queue(asafov::queue<std::string>& queue, const std::string& string)
{
    size_t checkpoint = 0;
    for (size_t pos = 0; pos < string.size(); pos++)
    {
        if (string[pos] == '(' || string[pos] == ')' || string[pos] == '+' || string[pos] == '-' ||
            string[pos] == '*' || string[pos] == '/' || string[pos] == '%')
        {
            queue.push(string.substr(pos, 1));
            checkpoint++;
        }
        else if (isdigit(string[pos-1]) && !isdigit(string[pos]))
        {
            queue.push(string.substr(checkpoint, pos-checkpoint));
            checkpoint++;
        }
        else if (isdigit(string[pos]))
        {
            pos = checkpoint + 1;
        }
        else
        {
            throw std::invalid_argument("invalid char!");
        }
    }
}

void asafov::into_polish(asafov::queue<std::string>& queue)
{
    
}

double asafov::count(asafov::queue<std::string>& queue)
{
    
}