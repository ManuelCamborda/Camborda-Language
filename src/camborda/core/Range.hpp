#pragma once

#include "Position.hpp"

#include <cstddef>
#include <string>

class Range {
public:
  Position start;
  Position end;

  Range(Position start, Position end);

  size_t length() const;
  bool empty() const;
  bool contains(const Position &p) const;
  bool contains(size_t offset) const;
  bool overlaps(const Range &other) const;

  static Range merge(const Range &a, const Range &b);

  bool operator==(const Range &other) const;
  bool operator!=(const Range &other) const;

  std::string toString() const;
};
