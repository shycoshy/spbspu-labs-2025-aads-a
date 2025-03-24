#include <iostream>
#include "countFunctions.h"

int main()
{
  std::string str;
  std::getline(std::cin, str);
  asafov::queue_t queue;
  asafov::str_to_queue(queue, str);
  asafov::into_polish(queue);
  std::cout << std::fixed << asafov::count(queue) << '\n';
  return 0;
}
