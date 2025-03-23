#include <iostream>
#include "parsingFunctions.h"
#include "queue.hpp"

int main()
{
  std::string str;
  std::cin >> str;
  asafov::queue<std::string> queue;
  asafov::str_to_queue(queue, str);
  /*while (!queue.empty())
  {
    std::cout << queue.drop() << std::endl;
  }*/
  asafov::into_polish(queue);
  while (!queue.empty())
  {
    std::cout << queue.drop() << std::endl;
  }
  //std::cout << asafov::count(queue) << '\n';  
  return 0;
}
