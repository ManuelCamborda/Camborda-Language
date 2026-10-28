#pragma once

#include "../../../core/Range.hpp"
#include "../expr/Expressions.hpp"
#include "../type/Types.hpp"
#include "Declaration.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Parameter {
  std::string name;
  TypePtr type;
  Range range;
};

class FunctionDecl final : public Declaration {
public:
  std::string name;
  std::vector<Parameter> params;
  TypePtr returnType;
  std::unique_ptr<BlockExpr> body;
  bool isExtern = false;
  bool isInline = false;

  FunctionDecl(std::string name, std::vector<Parameter> params,
               TypePtr returnType, std::unique_ptr<BlockExpr> body, Range range)
      : Declaration(DeclarationKind::DeclFunctionKind, std::move(range)),
        name(std::move(name)), params(std::move(params)),
        returnType(std::move(returnType)), body(std::move(body)) {}

  std::string toString() const override {
    return "FunctionDecl(" + name + ", " + std::to_string(params.size()) +
           " params)";
  }
};

class LetDecl final : public Declaration {
public:
  std::string name;
  TypePtr type;
  ExpressionPtr init;

  LetDecl(std::string name, TypePtr type, ExpressionPtr init, Range range)
      : Declaration(DeclarationKind::DeclLetKind, std::move(range)),
        name(std::move(name)), type(std::move(type)), init(std::move(init)) {}

  std::string toString() const override { return "LetDecl(" + name + ")"; }
};

class ConstDecl final : public Declaration {
public:
  std::string name;
  TypePtr type;
  ExpressionPtr init;

  ConstDecl(std::string name, TypePtr type, ExpressionPtr init, Range range)
      : Declaration(DeclarationKind::DeclConstKind, std::move(range)),
        name(std::move(name)), type(std::move(type)), init(std::move(init)) {}

  std::string toString() const override { return "ConstDecl(" + name + ")"; }
};
