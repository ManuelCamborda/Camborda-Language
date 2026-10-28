#pragma once

#include "../lexer/Lexer.hpp"
#include "../lexer/Token.hpp"
#include "expr/Expressions.hpp"
#include "type/Types.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

using DeclarationList = std::vector<DeclarationPtr>;

class Parser {
private:
  std::unique_ptr<Lexer> lexer;
  TokenList tokens;
  std::shared_ptr<std::string> file;
  size_t pos = 0;

  Token *peek(size_t offset = 0);
  Token *next();
  bool check(TokenKind kind, size_t offset = 0);
  Token *accept(TokenKind kind);
  Token *expect(TokenKind kind, const char *what);
  std::optional<Range> expectCloseAngle(const char *what);
  void error(Token *at, const std::string &message);
  void synchronize();

  TypePtr parseType();

  DeclarationPtr parseDecl();
  DeclarationPtr parseLetDecl();
  DeclarationPtr parseConstDecl();
  DeclarationPtr parseFunctionDecl();

  ExpressionPtr parseBlock();
  std::unique_ptr<BlockExpr> parseBlockBody();
  ExpressionPtr parseExpr();

  ExpressionPtr parseAdditive();
  ExpressionPtr parseMultiplicative();
  ExpressionPtr parseCast();
  ExpressionPtr parseUnary();
  ExpressionPtr parsePrimary();
  ExpressionPtr parseLiteral();

  ExpressionPtr parsePostfix(ExpressionPtr expr);

public:
  std::vector<std::string> errors;
  Parser(std::shared_ptr<std::string> file);

  DeclarationList parse(const char *src);
};
