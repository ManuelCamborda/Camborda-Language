#pragma once

#include "Token.hpp"

#include <cstdint>
#include <string>
#include <utility>

class IdentifierToken final : public Token {
public:
  IdentifierToken(std::string name, Range range)
      : Token(TokenKind::Identifier, std::move(name), std::move(range)) {}

  const std::string &name() const { return lexeme; }

  std::string toString() const override {
    return "IdentifierToken(" + name() + ")";
  }
};

class IntegerToken final : public Token {
public:
  uint64_t value;
  int base;
  std::string suffix; // "", "i32", "u8", ...

  IntegerToken(uint64_t value, int base, std::string suffix, std::string lexeme,
               Range range)
      : Token(TokenKind::IntLiteral, std::move(lexeme), std::move(range)),
        value(value), base(base), suffix(std::move(suffix)) {}

  std::string toString() const override {
    return "IntegerToken(" + std::to_string(value) + suffix + ")";
  }
};

class FloatToken final : public Token {
public:
  double value;
  std::string suffix; // "", "f32", "f64"

  FloatToken(double value, std::string suffix, std::string lexeme, Range range)
      : Token(TokenKind::FloatLiteral, std::move(lexeme), std::move(range)),
        value(value), suffix(std::move(suffix)) {}

  std::string toString() const override {
    return "FloatToken(" + std::to_string(value) + suffix + ")";
  }
};

class CharToken final : public Token {
public:
  char32_t value;

  CharToken(char32_t value, std::string lexeme, Range range)
      : Token(TokenKind::CharLiteral, std::move(lexeme), std::move(range)),
        value(value) {}

  std::string toString() const override {
    return "CharToken(U+" + std::to_string(static_cast<uint32_t>(value)) + ")";
  }
};

class StringToken final : public Token {
public:
  std::string value;

  StringToken(std::string value, std::string lexeme, Range range)
      : Token(TokenKind::StringLiteral, std::move(lexeme), std::move(range)),
        value(std::move(value)) {}

  std::string toString() const override {
    return "StringToken(\"" + value + "\")";
  }
};
