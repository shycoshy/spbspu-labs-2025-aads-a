#ifndef PARSING_FUNCTIONS_H
#define PARSING_FUNCTIONS_H

//очень удобный костыль, если я его не убрал а тесты проходятся то я лох
#define num_t unsigned long long

#include <string>
#include <stack>
#include <queue>

namespace asafov
{
  using stack_t = std::stack<std::string>;
  using queue_t = std::queue<std::string>;

  void str_to_queue(queue_t& queue, const std::string& string);
  void into_polish(queue_t& queue);
  num_t count(queue_t& queue);
}
#endif
