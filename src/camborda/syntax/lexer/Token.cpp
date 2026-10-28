#include "Token.hpp"

#include <utility>

Token::Token(TokenKind kind, std::string lexeme, Range range)
    : kind(kind), lexeme(std::move(lexeme)), range(std::move(range)) {}

bool Token::is(TokenKind k) const { return kind == k; }

bool Token::isOneOf(std::initializer_list<TokenKind> kinds) const {
  for (TokenKind k : kinds)
    if (kind == k)
      return true;
  return false;
}

bool Token::isKeyword() const {
  return kind >= TokenKind::KwLet && kind <= TokenKind::KwStr;
}

bool Token::isLiteral() const {
  return kind >= TokenKind::IntLiteral && kind <= TokenKind::StringLiteral;
}

bool Token::isAssignmentOperator() const {
  return kind >= TokenKind::Equal && kind <= TokenKind::QuestionQuestionEqual;
}

bool Token::isTrivia() const {
  return kind >= TokenKind::LineComment && kind <= TokenKind::Newline;
}

std::string Token::toString() const {
  return "Token(" + std::to_string(static_cast<int>(kind)) + ", \"" + lexeme +
         "\", " + range.toString() + ")";
}
