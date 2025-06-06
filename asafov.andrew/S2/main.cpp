#include <iostream>
#include <iomanip>
#include <fstream>
#include "countFunctions.h"

int main(int argc, char* argv[])
{
  asafov::stack_num_t result;
  if (argc == 1)
  {
    asafov::count(result, std::cin);
  }
  else if (argc == 2)
  {
    std::ifstream fin;
    fin.open(argv[1]);
    asafov::count(result, static_cast< std::istream >(fin));
  }
  else
  {
    std::cerr << "invalid arguments\n";
    return 1;
  }
}
