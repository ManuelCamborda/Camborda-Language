#pragma once

#include "../../core/Position.hpp"
#include "Token.hpp"

#include <memory>
#include <string>
#include <vector>

class Lexer {
public:
  explicit Lexer(std::shared_ptr<std::string> file);

  TokenList readTokens(std::string src);

  bool keepTrivia = false;
  std::vector<std::string> errors;

private:
  std::shared_ptr<std::string> file;
  Position _startPos;

  std::string _src;
  size_t _i = 0;
  Position _pos;

  bool atEnd() const;
  char peek(size_t n = 0) const;
  char advance();
  char32_t readCodePoint();

  std::string lexemeFrom(size_t startIdx) const;
  TokenPtr make(TokenKind kind, const Position &start, size_t startIdx);
  TokenPtr invalid(const Position &start, size_t startIdx,
                   const std::string &msg);
  void error(const Position &at, const std::string &msg);

  TokenPtr readNumber();
  TokenPtr readIdentifier();
  TokenPtr readString();
  TokenPtr readChar();
  TokenPtr readOperator();
  bool readEscape(char32_t &out);
};
