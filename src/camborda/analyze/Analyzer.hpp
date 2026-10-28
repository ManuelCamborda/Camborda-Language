#pragma once

#include "../syntax/ast/decl/Declaration.hpp"
#include "../syntax/ast/expr/Expressions.hpp"
#include "../syntax/ast/type/Types.hpp"

class Analyzer {
public:
  Analyzer();

  void analyze(DeclarationList &decls);

private:
  void analyzeDecl(Declaration *decl);
  TypePtr analyzeExpr(Expression *expr);

  void analyzeVarDecl(TypePtr type, Expression *defaultExpr, bool isConst);
};
