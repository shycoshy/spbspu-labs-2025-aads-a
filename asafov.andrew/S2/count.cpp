#include "countFunctions.h"
#include <stdexcept>
#include <cmath>

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

      num_t b = std::stod(stack.top());
      stack.pop();
      num_t a = std::stod(stack.top());
      stack.pop();
      num_t result = 0;

      if (token == "+")
      {
        result = a + b;
      }
      else if (token == "-")
      {
        result = a - b;
      }
      else if (token == "*")
      {
        result = a * b;
      }
      else if (token == "/")
      {
        if (b == 0.0)
        {
          throw std::logic_error("division by zero!");
        }
        result = a / b;
      }
      else if (token == "%")
      {
        result = std::fmod(a, b);
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

  return std::stod(stack.top());
}
