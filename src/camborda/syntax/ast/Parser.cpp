#include "Parser.hpp"
#include "../../core/Range.hpp"
#include "../lexer/Lexer.hpp"
#include "../lexer/Tokens.hpp"
#include "decl/Declarations.hpp"
#include "expr/Expressions.hpp"
#include "type/Types.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::string &textOf(const Token *t) { return t->lexeme; }

bool isPrimitiveTypeKw(TokenKind k) {
  return (k >= TokenKind::KwI8 && k <= TokenKind::KwStr) ||
         k == TokenKind::KwVoid;
}

} // namespace

Parser::Parser(std::shared_ptr<std::string> file)
    : file(std::move(file)), errors() {}

Token *Parser::peek(size_t offset) {
  size_t i = pos + offset;
  if (i >= tokens.size())
    return tokens.empty() ? nullptr : tokens.back().get();
  return tokens[i].get();
}

Token *Parser::next() {
  Token *tok = peek(0);
  if (pos < tokens.size())
    ++pos;
  return tok;
}

bool Parser::check(TokenKind kind, size_t offset) {
  Token *tok = peek(offset);
  return tok && tok->kind == kind;
}

Token *Parser::accept(TokenKind kind) { return check(kind) ? next() : nullptr; }

Token *Parser::expect(TokenKind kind, const char *what) {
  if (check(kind))
    return next();
  error(peek(), std::string("expected ") + what);
  return peek();
}

std::optional<Range> Parser::expectCloseAngle(const char *what) {
  Token *tok = peek();
  if (tok && tok->kind == TokenKind::Greater) {
    next();
    return tok->range;
  }
  if (tok && pos < tokens.size() &&
      (tok->kind == TokenKind::GreaterGreater ||
       tok->kind == TokenKind::GreaterEqual)) {
    Range r = tok->range;
    bool isShift = tok->kind == TokenKind::GreaterGreater;
    tokens[pos] =
        std::make_unique<Token>(isShift ? TokenKind::Greater : TokenKind::Equal,
                                isShift ? ">" : "=", r);
    return r;
  }
  error(tok, std::string("expected ") + what);
  return std::nullopt;
}

void Parser::error(Token *at, const std::string &message) {
  errors.push_back(at ? at->range.toString() + ": " + message : message);
}

void Parser::synchronize() {
  while (!check(TokenKind::EndOfFile)) {
    if (check(TokenKind::Semicolon)) {
      next();
      return;
    }
    if (check(TokenKind::RightBrace) || check(TokenKind::KwFunction) ||
        check(TokenKind::KwLet) || check(TokenKind::KwConst) ||
        check(TokenKind::KwExtern) || check(TokenKind::KwInline))
      return;
    next();
  }
}

TypePtr Parser::parseType() {
  Token *start = peek();

  if (accept(TokenKind::LeftBracket)) {
    TypePtr element = parseType();
    if (!element)
      return nullptr;

    std::optional<uint64_t> size;
    if (accept(TokenKind::Semicolon)) {
      Token *n = expect(TokenKind::IntLiteral, "array size");
      const auto *num = n->as<IntegerToken>();
      if (!num)
        return nullptr;
      size = num->value;
    }

    Token *end = expect(TokenKind::RightBracket, "']' to close array type");
    return std::make_unique<ArrayType>(std::move(element), size,
                                       Range::merge(start->range, end->range));
  }

  if (check(TokenKind::Identifier) ||
      (peek() && isPrimitiveTypeKw(peek()->kind))) {
    Token *nameTok = next();
    std::string name = textOf(nameTok);

    if (accept(TokenKind::Less)) {
      std::vector<TypePtr> args;
      while (true) {
        TypePtr arg = parseType();
        if (!arg)
          return nullptr;
        args.push_back(std::move(arg));
        if (!accept(TokenKind::Comma))
          break;
      }
      auto end = expectCloseAngle("'>' to close generic type");
      if (!end)
        return nullptr;
      return std::make_unique<GenericType>(std::move(name), std::move(args),
                                           Range::merge(nameTok->range, *end));
    }

    return std::make_unique<NamedType>(std::move(name), nameTok->range);
  }

  error(start, "expected type");
  return nullptr;
}

DeclarationPtr Parser::parseDecl() {
  Token *tok = peek();

  switch (tok->kind) {
  case TokenKind::KwLet:
    return parseLetDecl();
  case TokenKind::KwConst:
    return parseConstDecl();
  case TokenKind::KwFunction:
  case TokenKind::KwExtern:
  case TokenKind::KwInline:
    return parseFunctionDecl();
  default:
    error(tok, "expected declaration");
    return nullptr;
  }
}

DeclarationPtr Parser::parseLetDecl() {
  Token *start = next();

  Token *nameTok = expect(TokenKind::Identifier, "variable name");
  if (nameTok->kind != TokenKind::Identifier)
    return nullptr;

  TypePtr type;
  if (accept(TokenKind::Colon)) {
    type = parseType();
    if (!type)
      return nullptr;
  }

  ExpressionPtr init;
  if (accept(TokenKind::Equal)) {
    init = parseExpr();
    if (!init)
      return nullptr;
  }

  Token *end = expect(TokenKind::Semicolon, "';' after declaration");

  if (!type && !init)
    error(nameTok,
          "variable '" + textOf(nameTok) + "' needs a type or an initializer");

  return std::make_unique<LetDecl>(textOf(nameTok), std::move(type),
                                   std::move(init),
                                   Range::merge(start->range, end->range));
}

DeclarationPtr Parser::parseConstDecl() {
  Token *start = next();

  Token *nameTok = expect(TokenKind::Identifier, "constant name");
  if (nameTok->kind != TokenKind::Identifier)
    return nullptr;

  TypePtr type;
  if (accept(TokenKind::Colon)) {
    type = parseType();
    if (!type)
      return nullptr;
  }

  Token *eq = expect(TokenKind::Equal, "'=' (a constant needs an initializer)");
  if (eq->kind != TokenKind::Equal)
    return nullptr;

  ExpressionPtr init = parseExpr();
  if (!init)
    return nullptr;

  Token *end = expect(TokenKind::Semicolon, "';' after declaration");

  return std::make_unique<ConstDecl>(textOf(nameTok), std::move(type),
                                     std::move(init),
                                     Range::merge(start->range, end->range));
}

DeclarationPtr Parser::parseFunctionDecl() {
  Token *start = peek();

  bool isExtern = false;
  bool isInline = false;
  while (true) {
    if (accept(TokenKind::KwExtern))
      isExtern = true;
    else if (accept(TokenKind::KwInline))
      isInline = true;
    else
      break;
  }

  Token *kw = expect(TokenKind::KwFunction, "'function'");
  if (kw->kind != TokenKind::KwFunction)
    return nullptr;

  Token *nameTok = expect(TokenKind::Identifier, "function name");
  if (nameTok->kind != TokenKind::Identifier)
    return nullptr;

  Token *lp = expect(TokenKind::LeftParen, "'(' after function name");
  if (lp->kind != TokenKind::LeftParen)
    return nullptr;

  std::vector<Parameter> params;
  while (!check(TokenKind::RightParen) && !check(TokenKind::EndOfFile)) {
    Token *pStart = peek();

    Token *pName = expect(TokenKind::Identifier, "parameter name");
    if (pName->kind != TokenKind::Identifier)
      return nullptr;

    Token *colon = expect(TokenKind::Colon, "':' after parameter name");
    if (colon->kind != TokenKind::Colon)
      return nullptr;

    TypePtr type = parseType();
    if (!type)
      return nullptr;

    Range pRange = Range::merge(pStart->range, type->range);
    params.push_back(Parameter{textOf(pName), std::move(type), pRange});

    if (!accept(TokenKind::Comma))
      break;
  }

  Token *rp = expect(TokenKind::RightParen, "')' after parameters");
  if (rp->kind != TokenKind::RightParen)
    return nullptr;

  TypePtr returnType;
  if (accept(TokenKind::ThinArrow) || accept(TokenKind::Colon)) {
    returnType = parseType();
    if (!returnType)
      return nullptr;
  }

  std::unique_ptr<BlockExpr> body;
  Range endRange = rp->range;

  if (check(TokenKind::LeftBrace)) {
    body = parseBlockBody();
    if (!body)
      return nullptr;
    endRange = body->range;
  } else if (isExtern) {
    Token *semi = expect(TokenKind::Semicolon, "';' after extern function");
    endRange = semi->range;
  } else {
    error(peek(),
          "expected '{' to start the body of '" + textOf(nameTok) + "'");
    return nullptr;
  }

  auto decl = std::make_unique<FunctionDecl>(
      textOf(nameTok), std::move(params), std::move(returnType),
      std::move(body), Range::merge(start->range, endRange));
  decl->isExtern = isExtern;
  decl->isInline = isInline;
  return decl;
}

ExpressionPtr Parser::parseBlock() {
  if (check(TokenKind::LeftBrace))
    return parseBlockBody();
  return parseExpr();
}

std::unique_ptr<BlockExpr> Parser::parseBlockBody() {
  Token *start = expect(TokenKind::LeftBrace, "'{'");
  if (start->kind != TokenKind::LeftBrace)
    return nullptr;

  std::vector<ExpressionPtr> body;

  while (!check(TokenKind::RightBrace) && !check(TokenKind::EndOfFile)) {
    size_t before = pos;
    ExpressionPtr e;

    if (check(TokenKind::KwLet) || check(TokenKind::KwConst)) {
      DeclarationPtr d = parseDecl();
      if (d) {
        Range r = d->range;
        e = std::make_unique<DeclExpr>(std::move(d), r);
      }
    } else {
      e = parseExpr();

      if (e && !accept(TokenKind::Semicolon) && !check(TokenKind::RightBrace))
        error(peek(), "expected ';' after expression");
    }

    if (e)
      body.push_back(std::move(e));
    else
      synchronize();

    if (pos == before)
      next();
  }

  Token *end = expect(TokenKind::RightBrace, "'}' to close block");
  return std::make_unique<BlockExpr>(std::move(body),
                                     Range::merge(start->range, end->range));
}

ExpressionPtr Parser::parseExpr() {
  ExpressionPtr target = parseAdditive();
  if (!target)
    return nullptr;

  if (accept(TokenKind::Equal)) {
    ExpressionPtr value = parseExpr();
    if (!value)
      return nullptr;
    Range r = Range::merge(target->range, value->range);
    return std::make_unique<AssignExpr>(std::nullopt, std::move(target),
                                        std::move(value), r);
  }

  return target;
}

ExpressionPtr Parser::parseAdditive() {
  ExpressionPtr left = parseMultiplicative();
  if (!left)
    return nullptr;

  while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
    BinaryOp op =
        next()->kind == TokenKind::Plus ? BinaryOp::Add : BinaryOp::Sub;
    ExpressionPtr right = parseMultiplicative();
    if (!right)
      return nullptr;
    Range r = Range::merge(left->range, right->range);
    left =
        std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), r);
  }
  return left;
}

ExpressionPtr Parser::parseMultiplicative() {
  ExpressionPtr left = parseCast();
  if (!left)
    return nullptr;

  while (check(TokenKind::Star) || check(TokenKind::Slash) ||
         check(TokenKind::Percent)) {
    TokenKind k = next()->kind;
    BinaryOp op = k == TokenKind::Star    ? BinaryOp::Mul
                  : k == TokenKind::Slash ? BinaryOp::Div
                                          : BinaryOp::Mod;
    ExpressionPtr right = parseCast();
    if (!right)
      return nullptr;
    Range r = Range::merge(left->range, right->range);
    left =
        std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), r);
  }
  return left;
}

ExpressionPtr Parser::parseCast() {
  ExpressionPtr expr = parseUnary();

  while (expr && accept(TokenKind::KwAs)) {
    TypePtr type = parseType();
    if (!type)
      return nullptr;
    Range r = Range::merge(expr->range, type->range);
    expr = std::make_unique<CastExpr>(std::move(expr), std::move(type), r);
  }
  return expr;
}

ExpressionPtr Parser::parseUnary() {
  if (check(TokenKind::Minus) || check(TokenKind::Bang)) {
    Token *opTok = next();
    UnaryOp op =
        opTok->kind == TokenKind::Minus ? UnaryOp::Negate : UnaryOp::Not;
    ExpressionPtr operand = parseUnary();
    if (!operand)
      return nullptr;
    Range r = Range::merge(opTok->range, operand->range);
    return std::make_unique<UnaryExpr>(op, std::move(operand), r);
  }

  ExpressionPtr primary = parsePrimary();
  if (!primary)
    return nullptr;
  return parsePostfix(std::move(primary));
}

ExpressionPtr Parser::parsePrimary() {
  if (ExpressionPtr lit = parseLiteral())
    return lit;

  Token *tok = peek();

  switch (tok->kind) {
  case TokenKind::Identifier:
    next();
    return std::make_unique<IdentifierExpr>(textOf(tok), tok->range);

  case TokenKind::LeftParen: {
    next();
    ExpressionPtr inner = parseExpr();
    if (!inner)
      return nullptr;
    expect(TokenKind::RightParen, "')'");
    return inner;
  }

  case TokenKind::KwSizeof: {
    next();
    Token *lp = expect(TokenKind::LeftParen, "'(' after sizeof");
    if (lp->kind != TokenKind::LeftParen)
      return nullptr;
    TypePtr type = parseType();
    if (!type)
      return nullptr;
    Token *rp = expect(TokenKind::RightParen, "')' after sizeof type");
    return std::make_unique<SizeofExpr>(std::move(type),
                                        Range::merge(tok->range, rp->range));
  }

  case TokenKind::LeftBrace:
    return parseBlock();

  case TokenKind::KwReturn: {
    next();
    ExpressionPtr value;
    if (!check(TokenKind::Semicolon) && !check(TokenKind::RightBrace)) {
      value = parseExpr();
      if (!value)
        return nullptr;
    }
    Range r = value ? Range::merge(tok->range, value->range) : tok->range;
    return std::make_unique<ReturnExpr>(std::move(value), r);
  }

  default:
    error(tok, "expected expression");
    return nullptr;
  }
}

ExpressionPtr Parser::parseLiteral() {
  Token *tok = peek();
  if (!tok)
    return nullptr;

  switch (tok->kind) {
  case TokenKind::IntLiteral: {
    const auto *t = tok->as<IntegerToken>();
    if (!t)
      return nullptr;
    next();
    return std::make_unique<IntLiteralExpr>(t->value, t->suffix, tok->range);
  }
  case TokenKind::FloatLiteral: {
    const auto *t = tok->as<FloatToken>();
    if (!t)
      return nullptr;
    next();
    return std::make_unique<FloatLiteralExpr>(t->value, t->suffix, tok->range);
  }
  case TokenKind::StringLiteral: {
    const auto *t = tok->as<StringToken>();
    if (!t)
      return nullptr;
    next();
    return std::make_unique<StringLiteralExpr>(t->value, tok->range);
  }
  case TokenKind::CharLiteral: {
    const auto *t = tok->as<CharToken>();
    if (!t)
      return nullptr;
    next();
    return std::make_unique<CharLiteralExpr>(t->value, tok->range);
  }
  case TokenKind::KwTrue:
    next();
    return std::make_unique<BoolLiteralExpr>(true, tok->range);
  case TokenKind::KwFalse:
    next();
    return std::make_unique<BoolLiteralExpr>(false, tok->range);
  case TokenKind::KwNull:
    next();
    return std::make_unique<NullLiteralExpr>(tok->range);
  default:
    return nullptr;
  }
}

ExpressionPtr Parser::parsePostfix(ExpressionPtr expr) {
  while (true) {
    if (accept(TokenKind::LeftParen)) {
      std::vector<ExpressionPtr> args;
      while (!check(TokenKind::RightParen) && !check(TokenKind::EndOfFile)) {
        ExpressionPtr arg = parseExpr();
        if (!arg)
          return nullptr;
        args.push_back(std::move(arg));
        if (!accept(TokenKind::Comma))
          break;
      }
      Token *end = expect(TokenKind::RightParen, "')' after arguments");
      Range r = Range::merge(expr->range, end->range);
      expr = std::make_unique<CallExpr>(std::move(expr), std::move(args), r);

    } else if (accept(TokenKind::LeftBracket)) {
      ExpressionPtr index = parseExpr();
      if (!index)
        return nullptr;
      Token *end = expect(TokenKind::RightBracket, "']' after index");
      Range r = Range::merge(expr->range, end->range);
      expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index), r);

    } else if (check(TokenKind::Dot) || check(TokenKind::ThinArrow)) {
      bool viaPointer = next()->kind == TokenKind::ThinArrow;
      Token *name = expect(TokenKind::Identifier, "member name");
      if (name->kind != TokenKind::Identifier)
        return nullptr;
      Range r = Range::merge(expr->range, name->range);
      expr = std::make_unique<MemberExpr>(std::move(expr), textOf(name),
                                          viaPointer, r);
    } else {
      break;
    }
  }
  return expr;
}

DeclarationList Parser::parse(const char *src) {
  this->lexer = std::make_unique<Lexer>(file);
  lexer->keepTrivia = false;
  this->tokens = lexer->readTokens(src);
  this->pos = 0;
  this->errors = lexer->errors;

  DeclarationList decls;
  if (tokens.empty())
    return decls;

  while (pos < tokens.size() && !check(TokenKind::EndOfFile)) {
    size_t before = pos;

    DeclarationPtr decl = parseDecl();
    if (decl)
      decls.push_back(std::move(decl));
    else
      synchronize();

    if (pos == before)
      next();
  }

  return decls;
}
