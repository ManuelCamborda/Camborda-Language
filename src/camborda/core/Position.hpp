#pragma once

#include <cstddef>
#include <memory>
#include <string>

class Position {
public:
  size_t line;
  size_t column;
  size_t offset;
  std::shared_ptr<const std::string> file;

  Position(size_t line, size_t column, size_t offset,
           std::shared_ptr<const std::string> file = nullptr);

  Position advance(char c) const;

  bool operator==(const Position &other) const;
  bool operator!=(const Position &other) const;
  bool operator<(const Position &other) const;
  bool operator<=(const Position &other) const;

  std::string toString() const;
};
