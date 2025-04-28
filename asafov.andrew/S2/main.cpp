#include <iostream>
#include <iomanip>
#include <fstream>
#include "countFunctions.h"

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    try
    {
      std::string str;
      std::getline(std::cin, str);
      asafov::queue_t queue;
      asafov::str_to_queue(queue, str);
      asafov::into_polish(queue);
      std::cout << std::fixed << std::setprecision(0) << asafov::count(queue) << '\n';
    }
    catch (const std::logic_error& e)
    {
      return 0;
    }
    catch (const std::invalid_argument& e)
    {
      std::cerr << e.what() << '\n';
      return 1;
    }
  }
  else if (argc == 2)
  {
    std::ifstream fin(argv[1]);
    std::stack<num_t> result; 
    while (!fin.eof())
    {
      
      try
      {
        std::string str;
        std::getline(std::cin, str);
        asafov::queue_t queue;
        asafov::str_to_queue(queue, str);
        asafov::into_polish(queue);
        result.push(asafov::count(queue));
      }
      catch (const std::logic_error& e)
      {
        continue;
      }
      catch (const std::invalid_argument& e)
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
