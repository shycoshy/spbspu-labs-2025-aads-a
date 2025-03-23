#ifndef PARSING_FUNCTIONS_H
#define PARSING_FUNCTIONS_H
#include <string>
#include "queue.hpp"

namespace asafov
{
  void str_to_queue(asafov::queue<std::string>& queue, const std::string& string);
  void into_polish(asafov::queue<std::string>& queue);
  double count(asafov::queue<std::string>& queue);
}
#endif
