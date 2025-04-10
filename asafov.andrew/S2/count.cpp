#include "countFunctions.h"
#include <stdexcept>
#include <cmath>
//#include <iostream>

namespace
{
  bool is_operator(const std::string& s)
  {
    return s == "+" || s == "-" || s == "*" || s == "/" || s == "%";
  }
}

num_t asafov::count(queue_t& queue)
{
  stack_t stack;

  while (!queue.empty())
  {
    std::string token = queue.front();
    queue.pop();

    if (is_operator(token))
    {
      if (stack.size() < 2)
      {
        throw std::logic_error("not enough operands!");
      }
      num_t b = std::stoll(stack.top());
      //std::clog << "b(" << stack.top() << ") = " << b << std::endl;
      stack.pop();
      num_t a = std::stoll(stack.top());
      //std::clog << "a(" << stack.top() << ") = " << a << std::endl;
      stack.pop();
      num_t result = 0;

      if (token == "+")
      {
        result = a + b;
        //std::clog << "a(" << a << ") + b(" << b << ") = result(" << result << ')' << std::endl;
      }
      else if (token == "-")
      {
        result = a - b;
        //std::clog << "a(" << a << ") - b(" << b << ") = result(" << result << ')' << std::endl;
        if (result < -std::numeric_limits<num_t>::max())
        {
          throw std::logic_error("underflow!");
        }
      }
      else if (token == "*")
      {
        result = a * b;
        //std::clog << "a(" << a << ") * b(" << b << ") = result(" << result << ')' << std::endl;
      }
      else if (token == "/")
      {
        if (b == 0.0)
        {
          throw std::logic_error("division by zero!");
        }
        result = a / b;
        //std::clog << "a(" << a << ") / b(" << b << ") = result(" << result << ')' << std::endl;
      }
      else if (token == "%")
      {
        result = std::fmod(a, b);
        //std::clog << "a(" << a << ") % b(" << b << ") = result(" << result << ')' << std::endl;
      }
      stack.push(std::to_string(result));
    }
    else
    {
      stack.push(token);
    }
  }

  if (stack.size() != 1)
  {
    while (!stack.empty())
    {
      stack.pop();
    }
    throw std::logic_error("invalid expression!");
  }

  return std::stoll(stack.top());
}
