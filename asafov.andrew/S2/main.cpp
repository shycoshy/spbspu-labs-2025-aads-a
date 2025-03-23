#include <iostream>
#include "parsingFunctions.h"
#include "queue.hpp"

int main()
{
  std::string str;
  std::cin >> str;
  asafov::queue<std::string> queue;
  asafov::str_to_queue(queue, str);
  asafov::into_polish(queue);
  std::cout << asafov::count(queue) << '\n';  
  return 0;
}
