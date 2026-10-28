#include "Range.hpp"

Range::Range(Position start, Position end)
    : start(std::move(start)), end(std::move(end)) {}

size_t Range::length() const { return end.offset - start.offset; }

bool Range::empty() const { return start.offset == end.offset; }

bool Range::contains(const Position &p) const { return contains(p.offset); }

bool Range::contains(size_t offset) const {
  return offset >= start.offset && offset < end.offset;
}

bool Range::overlaps(const Range &other) const {
  return start.offset < other.end.offset && other.start.offset < end.offset;
}

Range Range::merge(const Range &a, const Range &b) {
  return Range(a.start < b.start ? a.start : b.start,
               a.end < b.end ? b.end : a.end);
}

bool Range::operator==(const Range &other) const {
  return start == other.start && end == other.end;
}

bool Range::operator!=(const Range &other) const { return !(*this == other); }

std::string Range::toString() const {
  return start.toString() + "-" + std::to_string(end.line) + ":" +
         std::to_string(end.column);
}
