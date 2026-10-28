#include "Position.hpp"

#include <utility>

Position::Position(size_t line, size_t column, size_t offset,
                   std::shared_ptr<const std::string> file)
    : line(line), column(column), offset(offset), file(std::move(file)) {}

Position Position::advance(char c) const {
  Position next = *this;
  next.offset++;
  if (c == '\n') {
    next.line++;
    next.column = 1;
  } else if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) {
    next.column++;
  }
  return next;
}

bool Position::operator==(const Position &other) const {
  return offset == other.offset && file == other.file;
}

bool Position::operator!=(const Position &other) const {
  return !(*this == other);
}

bool Position::operator<(const Position &other) const {
  return offset < other.offset;
}

bool Position::operator<=(const Position &other) const {
  return offset <= other.offset;
}

std::string Position::toString() const {
  return (file ? *file : std::string("<unknown>")) + ":" +
         std::to_string(line) + ":" + std::to_string(column);
}
