#include "command.h"
#include <iostream>
#include <sstream>

void asafov::printMap(const maps_t& datasets, const std::string& name)
{
  auto it = datasets.find(name);
  if (it == datasets.end() || (*it).second.empty())
  {
    std::cout << "<EMPTY>\n";
    return;
  }

  std::cout << name;
  for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
  {
    auto [key, value] = *it2;
    std::cout << " " << key << " " << value;
  }
  std::cout << '\n';
}

bool asafov::complement(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right)
{
  if (datasets.find(left) == datasets.end() || datasets.find(right) == datasets.end())
  {
    return false;
  }

  map_t result;
  const map_t& leftMap = datasets[left];
  const map_t& rightMap = datasets[right];

  for (auto it = leftMap.begin(); it != leftMap.end(); ++it)
  {
    auto [key, value] = *it;
    if (rightMap.find(key) == rightMap.end())
    {
      result[key] = value;
    }
  }

  datasets[newName] = result;
  return true;
}

bool asafov::intersect(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right)
{
  if (datasets.find(left) == datasets.end() || datasets.find(right) == datasets.end())
  {
    return false;
  }

  map_t result;
  const map_t& leftMap = datasets[left];
  const map_t& rightMap = datasets[right];

  for (auto it = leftMap.begin(); it != leftMap.end(); ++it)
  {
    auto [key, value] = *it;
    if (rightMap.find(key) != rightMap.end())
    {
      result[key] = value;
    }
  }

  datasets[newName] = result;
  return true;
}

bool asafov::unionMaps(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right)
{
  if (datasets.find(left) == datasets.end() || datasets.find(right) == datasets.end())
  {
    return false;
  }

  map_t result = datasets[left];
  const map_t& rightMap = datasets[right];

  for (auto it = rightMap.begin(); it != rightMap.end(); ++it)
  {
    auto [key, value] = *it;
    if (result.find(key) == result.end())
    {
      result[key] = value;
    }
  }

  datasets[newName] = result;
  return true;
}

bool asafov::processCommand(maps_t& datasets, const std::string& line)
{
  std::istringstream ss(line);
  std::string command;
  ss >> command;

  if (command == "print")
  {
    std::string name;
    if (ss >> name)
    {
      printMap(datasets, name);
      return true;
    }
  }
  else if (command == "complement")
  {
    std::string newName, left, right;
    if (ss >> newName >> left >> right)
    {
      if (!complement(datasets, newName, left, right))
      {
        std::cout << "invalid command!\n";
      }
      return true;
    }
  }
  else if (command == "intersect")
  {
    std::string newName, left, right;
    if (ss >> newName >> left >> right)
    {
      if (!intersect(datasets, newName, left, right))
      {
        std::cout << "invalid command!\n";
      }
      return true;
    }
  }
  else if (command == "union")
  {
    std::string newName, left, right;
    if (ss >> newName >> left >> right)
    {
      if (!unionMaps(datasets, newName, left, right))
      {
        std::cout << "invalid command!\n";
      }
      return true;
    }
  }

  std::cout << "invalid command!\n";
  return false;
}
