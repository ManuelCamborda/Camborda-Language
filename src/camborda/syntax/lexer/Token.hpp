#pragma once

#include "../../core/Range.hpp"
#include "TokenKind.hpp"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

class Token {
public:
  TokenKind kind;
  std::string lexeme;
  Range range;

  Token(TokenKind kind, std::string lexeme, Range range);
  virtual ~Token() = default;

  bool is(TokenKind k) const;
  bool isOneOf(std::initializer_list<TokenKind> kinds) const;

  bool isKeyword() const;
  bool isLiteral() const;
  bool isAssignmentOperator() const;
  bool isTrivia() const;

  template <typename T> const T *as() const {
    return dynamic_cast<const T *>(this);
  }
  template <typename T> bool isA() const { return as<T>() != nullptr; }

  virtual std::string toString() const;
};

using TokenPtr = std::unique_ptr<Token>;
using TokenList = std::vector<TokenPtr>;
