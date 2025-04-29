#include <iostream>
#include <iomanip>
#include <fstream>
#include "countFunctions.h"

namespace
{
  int countExpression(std::istream& is, std::stack<num_t>& result)
  {
    std::string str;
    std::getline(is, str);
    if (str.empty()) return 0;
    asafov::queue_t queue;
    asafov::str_to_queue(queue, str);
    try
    {
      asafov::into_polish(queue);
      result.push(asafov::count(queue));
    }
    catch (const std::invalid_argument& e)
    {
      std::cerr << e.what() << '\n';
      return 1;
    }
    catch (const std::logic_error& e)
    {
      std::cerr << e.what() << '\n';
      return 2;
    }
    return 0;
  }
}

int main(int argc, char* argv[])
{
  std::stack<num_t> result;
  std::istream* is = nullptr;
  if (argc == 1)
  {
    is = &std::cin;
  }
  else if (argc == 2)
  {
    std::ifstream fin(argv[1]);
    is = &fin;
  }
  else
  {
    std::cerr << "invalid arguments\n";
    return 1;
  }

  while (!is->eof())
  {
    int error = countExpression(*is, result);
    if (error == 1) continue;
    else if (error == 2) return 1;
  }
  if (result.empty())
  {
    std::cout << '\n';
    return 0;
  }
  while (result.size() > 1)
  {
    std::cout << std::fixed << std::setprecision(0) << result.top() << ' ';
    result.pop();
  }
  std::cout << std::fixed << std::setprecision(0) << result.top() << '\n';
  result.pop();
  return 0;
}
