#include <iostream>
#include <iomanip>
#include <fstream>
#include "countFunctions.h"

namespace
{
  int countExpression(std::istream& is, std::stack<num_t>& result)
  {
    std::string str;
    std::getline(std::cin, str);
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
      return 1;
    }
  }
}

int main(int argc, char* argv[])
{
  std::stack<num_t> result;
  if (argc == 1)
  {
    if (countExpression(std::cin, result)) return 1;
    std::cout << std::fixed << std::setprecision(0) << result.top() << '\n';
    result.pop();
  }
  else if (argc == 2)
  {
    std::ifstream fin(argv[1]);
    if (!fin.eof())
    {
      std::cout << '\n';
       return 0;
    }
    while (!fin.eof())
    {
      std::string str;
      std::getline(std::cin, str);
      if (str.empty()) continue;
      asafov::queue_t queue;
      asafov::str_to_queue(queue, str);
      try
      {
        asafov::into_polish(queue);
        result.push(asafov::count(queue));
      }
      catch (const std::invalid_argument& e)
      {
        continue;
      }
      catch (const std::logic_error& e)
      {
       std::cerr << e.what() << '\n';
       return 1;
      }
    }
    fin.close();
    if (result.empty())
    {
      std::cout << '\n';
    }
    else if (result.size() > 1)
    {
      std::cout << result.top() << '\n';
    }
    else
    {
      std::cout << std::fixed << std::setprecision(0);
      std::cout << result.top();
      result.pop();
      while (!result.empty())
      {
        std::cout << ' ' << result.top();
        result.pop();
      }
      std::cout << '\n';
    }
    return 0;
  }
  else
  {
    std::cerr << "invalid arguments\n";
    return 1;
  }
}
