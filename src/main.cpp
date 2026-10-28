#include "camborda/syntax/ast/Parser.hpp"
#include "camborda/syntax/ast/decl/Declarations.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

static void dump(const Declaration &d) {
  if (auto *f = d.as<FunctionDecl>()) {
    std::cout << (f->isExtern ? "extern " : "")
              << (f->isInline ? "inline " : "") << "function " << f->name
              << "(";
    for (size_t i = 0; i < f->params.size(); i++) {
      const Parameter &p = f->params[i];
      std::cout << (i ? ", " : "") << p.name << ": " << p.type->toString();
    }
    std::cout << ") -> "
              << (f->returnType ? f->returnType->toString() : "void");
    if (f->body) {
      std::cout << " {\n";
      for (auto &e : f->body->body)
        std::cout << "  " << e->toString() << "\n";
      std::cout << "}";
    }
    std::cout << "\n";
  } else if (auto *l = d.as<LetDecl>()) {
    std::cout << "let " << l->name << ": "
              << (l->type ? l->type->toString() : "<inferred>")
              << (l->init ? " = " + l->init->toString() : "") << "\n";
  } else if (auto *c = d.as<ConstDecl>()) {
    std::cout << "const " << c->name << ": "
              << (c->type ? c->type->toString() : "<inferred>") << " = "
              << c->init->toString() << "\n";
  } else {
    std::cout << d.toString() << "\n";
  }
}

int main(int, char **) {
  std::ifstream t("test.cam");
  std::stringstream buffer;
  buffer << t.rdbuf();

  Parser parser(std::make_shared<std::string>("main.cam"));

  DeclarationList decls = parser.parse(buffer.str().c_str());

  for (auto &decl : decls)
    if (decl)
      dump(*decl);

  for (auto &err : parser.errors)
    std::cerr << "error: " << err << std::endl;

  return parser.errors.empty() ? 0 : 1;
}
