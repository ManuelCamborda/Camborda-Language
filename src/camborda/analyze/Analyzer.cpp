#include "Analyzer.hpp"
#include <memory>

Analyzer::Analyzer() {}

void Analyzer::analyze(DeclarationList &decls) {
  for (auto &decl : decls) {
    analyzeDecl(decl.get());
  }
}

void Analyzer::analyzeDecl(Declaration *decl) {
  if (decl->is(DeclarationKind::DeclLetKind)) {
  } else if (decl->is(DeclarationKind::DeclConstKind)) {
  } else if (decl->is(DeclarationKind::DeclFunctionKind)) {
  }
}

void Analyzer::analyzeVarDecl(TypePtr type, Expression *defaultExpr,
                              bool isConst) {
  if (type == nullptr) {
    type = std::move(analyzeExpr(defaultExpr));
  }
}
