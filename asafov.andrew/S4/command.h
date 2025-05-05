#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include <map>
namespace asafov
{
  using map_t = std::map<int, std::string>;
  using maps_t = std::map<std::string, map_t>;

  void printMap(const maps_t& datasets, const std::string& name);
  bool complement(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right);
  bool intersect(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right);
  bool unionMaps(maps_t& datasets, const std::string& newName, const std::string& left, const std::string& right);
  bool processCommand(maps_t& datasets, const std::string& line);
}
#endif
