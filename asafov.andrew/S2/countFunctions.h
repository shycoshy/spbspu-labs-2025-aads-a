#ifndef PARSING_FUNCTIONS_H
#define PARSING_FUNCTIONS_H

#include <string>
#include <iostream>
#include <stack.hpp>
#include <queue.hpp>

namespace asafov
{
  using num_t = long long;
  using stack_t = asafov::stack<std::string>;
  using queue_t = asafov::queue<std::string>;

  void parseStringToQueue(queue_t& queue, const std::string& string);
  void convertInfixToPostfix(queue_t& queue);
  num_t evaluatePostfixExpression(queue_t& queue);
  int evaluateExpressionFromString(std::istream& is, asafov::stack<num_t>& result);
}
#endif
