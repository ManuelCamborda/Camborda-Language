#pragma once

#include "../../../core/Range.hpp"
#include "../decl/Declaration.hpp"
#include "../type/Types.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class UnaryOp {
  Negate,    // -x
  Not,       // !x
  BitNot,    // ~x
  Deref,     // *x
  AddressOf, // &x
  PreInc,    // ++x
  PreDec,    // --x
  PostInc,   // x++
  PostDec    // x--
};

enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Pow,
  Shl,
  Shr,
  BitAnd,
  BitOr,
  BitXor,
  And,
  Or,
  Eq,
  Ne,
  Lt,
  Le,
  Gt,
  Ge
};

inline const char *toString(UnaryOp op) {
  switch (op) {
  case UnaryOp::Negate:
    return "-";
  case UnaryOp::Not:
    return "!";
  case UnaryOp::BitNot:
    return "~";
  case UnaryOp::Deref:
    return "*";
  case UnaryOp::AddressOf:
    return "&";
  case UnaryOp::PreInc:
    return "++(pre)";
  case UnaryOp::PreDec:
    return "--(pre)";
  case UnaryOp::PostInc:
    return "++(post)";
  case UnaryOp::PostDec:
    return "--(post)";
  }
  return "?";
}

inline const char *toString(BinaryOp op) {
  switch (op) {
  case BinaryOp::Add:
    return "+";
  case BinaryOp::Sub:
    return "-";
  case BinaryOp::Mul:
    return "*";
  case BinaryOp::Div:
    return "/";
  case BinaryOp::Mod:
    return "%";
  case BinaryOp::Pow:
    return "**";
  case BinaryOp::Shl:
    return "<<";
  case BinaryOp::Shr:
    return ">>";
  case BinaryOp::BitAnd:
    return "&";
  case BinaryOp::BitOr:
    return "|";
  case BinaryOp::BitXor:
    return "^";
  case BinaryOp::And:
    return "&&";
  case BinaryOp::Or:
    return "||";
  case BinaryOp::Eq:
    return "==";
  case BinaryOp::Ne:
    return "!=";
  case BinaryOp::Lt:
    return "<";
  case BinaryOp::Le:
    return "<=";
  case BinaryOp::Gt:
    return ">";
  case BinaryOp::Ge:
    return ">=";
  }
  return "?";
}

enum class ExpressionKind {
  ExprIntLiteralKind,
  ExprFloatLiteralKind,
  ExprStringLiteralKind,
  ExprCharLiteralKind,
  ExprBoolLiteralKind,
  ExprNullLiteralKind,
  ExprIdentifierKind,
  ExprUnaryKind,
  ExprBinaryKind,
  ExprAssignKind,
  ExprCallKind,
  ExprIndexKind,
  ExprMemberKind,
  ExprCastKind,
  ExprSizeofKind,
  ExprConditionalKind,
  ExprBlockKind,
  ExprDeclKind,
  ExprIfKind,
  ExprWhileKind,
  ExprDoWhileKind,
  ExprForKind,
  ExprReturnKind,
  ExprBreakKind,
  ExprContinueKind
};

class Expression {
public:
  ExpressionKind kind;
  Range range;

  Expression(ExpressionKind kind, Range range)
      : kind(kind), range(std::move(range)) {}
  virtual ~Expression() = default;

  bool is(ExpressionKind k) const { return kind == k; }

  template <typename T> const T *as() const {
    return dynamic_cast<const T *>(this);
  }
  template <typename T> bool isA() const { return as<T>() != nullptr; }

  virtual std::string toString() const = 0;
};

using ExpressionPtr = std::unique_ptr<Expression>;

class IntLiteralExpr final : public Expression {
public:
  uint64_t value;
  std::string suffix;

  IntLiteralExpr(uint64_t value, std::string suffix, Range range)
      : Expression(ExpressionKind::ExprIntLiteralKind, std::move(range)),
        value(value), suffix(std::move(suffix)) {}

  std::string toString() const override {
    return std::to_string(value) + suffix;
  }
};

class FloatLiteralExpr final : public Expression {
public:
  double value;
  std::string suffix;

  FloatLiteralExpr(double value, std::string suffix, Range range)
      : Expression(ExpressionKind::ExprFloatLiteralKind, std::move(range)),
        value(value), suffix(std::move(suffix)) {}

  std::string toString() const override {
    return std::to_string(value) + suffix;
  }
};

class StringLiteralExpr final : public Expression {
public:
  std::string value;

  StringLiteralExpr(std::string value, Range range)
      : Expression(ExpressionKind::ExprStringLiteralKind, std::move(range)),
        value(std::move(value)) {}

  std::string toString() const override { return "\"" + value + "\""; }
};

class CharLiteralExpr final : public Expression {
public:
  char32_t value;

  CharLiteralExpr(char32_t value, Range range)
      : Expression(ExpressionKind::ExprCharLiteralKind, std::move(range)),
        value(value) {}

  std::string toString() const override {
    return "char(" + std::to_string(static_cast<uint32_t>(value)) + ")";
  }
};

class BoolLiteralExpr final : public Expression {
public:
  bool value;

  BoolLiteralExpr(bool value, Range range)
      : Expression(ExpressionKind::ExprBoolLiteralKind, std::move(range)),
        value(value) {}

  std::string toString() const override { return value ? "true" : "false"; }
};

class NullLiteralExpr final : public Expression {
public:
  explicit NullLiteralExpr(Range range)
      : Expression(ExpressionKind::ExprNullLiteralKind, std::move(range)) {}

  std::string toString() const override { return "null"; }
};

class IdentifierExpr final : public Expression {
public:
  std::string name;

  IdentifierExpr(std::string name, Range range)
      : Expression(ExpressionKind::ExprIdentifierKind, std::move(range)),
        name(std::move(name)) {}

  std::string toString() const override { return name; }
};

class UnaryExpr final : public Expression {
public:
  UnaryOp op;
  ExpressionPtr operand;

  UnaryExpr(UnaryOp op, ExpressionPtr operand, Range range)
      : Expression(ExpressionKind::ExprUnaryKind, std::move(range)), op(op),
        operand(std::move(operand)) {}

  std::string toString() const override {
    return std::string("(") + ::toString(op) + " " + operand->toString() + ")";
  }
};

class BinaryExpr final : public Expression {
public:
  BinaryOp op;
  ExpressionPtr left;
  ExpressionPtr right;

  BinaryExpr(BinaryOp op, ExpressionPtr left, ExpressionPtr right, Range range)
      : Expression(ExpressionKind::ExprBinaryKind, std::move(range)), op(op),
        left(std::move(left)), right(std::move(right)) {}

  std::string toString() const override {
    return "(" + left->toString() + " " + ::toString(op) + " " +
           right->toString() + ")";
  }
};

class AssignExpr final : public Expression {
public:
  std::optional<BinaryOp> op;
  ExpressionPtr target;
  ExpressionPtr value;

  AssignExpr(std::optional<BinaryOp> op, ExpressionPtr target,
             ExpressionPtr value, Range range)
      : Expression(ExpressionKind::ExprAssignKind, std::move(range)), op(op),
        target(std::move(target)), value(std::move(value)) {}

  std::string toString() const override {
    return "(" + target->toString() + " " + (op ? ::toString(*op) : "") + "= " +
           value->toString() + ")";
  }
};

class ConditionalExpr final : public Expression {
public:
  ExpressionPtr condition;
  ExpressionPtr thenExpr;
  ExpressionPtr elseExpr;

  ConditionalExpr(ExpressionPtr condition, ExpressionPtr thenExpr,
                  ExpressionPtr elseExpr, Range range)
      : Expression(ExpressionKind::ExprConditionalKind, std::move(range)),
        condition(std::move(condition)), thenExpr(std::move(thenExpr)),
        elseExpr(std::move(elseExpr)) {}

  std::string toString() const override {
    return "(" + condition->toString() + " ? " + thenExpr->toString() + " : " +
           elseExpr->toString() + ")";
  }
};

class CallExpr final : public Expression {
public:
  ExpressionPtr callee;
  std::vector<ExpressionPtr> args;

  CallExpr(ExpressionPtr callee, std::vector<ExpressionPtr> args, Range range)
      : Expression(ExpressionKind::ExprCallKind, std::move(range)),
        callee(std::move(callee)), args(std::move(args)) {}

  std::string toString() const override {
    return "call(" + callee->toString() + ", " + std::to_string(args.size()) +
           " args)";
  }
};

class IndexExpr final : public Expression {
public:
  ExpressionPtr object;
  ExpressionPtr index;

  IndexExpr(ExpressionPtr object, ExpressionPtr index, Range range)
      : Expression(ExpressionKind::ExprIndexKind, std::move(range)),
        object(std::move(object)), index(std::move(index)) {}

  std::string toString() const override {
    return object->toString() + "[" + index->toString() + "]";
  }
};

class MemberExpr final : public Expression {
public:
  ExpressionPtr object;
  std::string member;
  bool viaPointer;

  MemberExpr(ExpressionPtr object, std::string member, bool viaPointer,
             Range range)
      : Expression(ExpressionKind::ExprMemberKind, std::move(range)),
        object(std::move(object)), member(std::move(member)),
        viaPointer(viaPointer) {}

  std::string toString() const override {
    return object->toString() + (viaPointer ? "->" : ".") + member;
  }
};

class CastExpr final : public Expression {
public:
  ExpressionPtr expr;
  TypePtr type;

  CastExpr(ExpressionPtr expr, TypePtr type, Range range)
      : Expression(ExpressionKind::ExprCastKind, std::move(range)),
        expr(std::move(expr)), type(std::move(type)) {}

  std::string toString() const override {
    return "(" + expr->toString() + " as " + type->toString() + ")";
  }
};

class SizeofExpr final : public Expression {
public:
  TypePtr type;

  SizeofExpr(TypePtr type, Range range)
      : Expression(ExpressionKind::ExprSizeofKind, std::move(range)),
        type(std::move(type)) {}

  std::string toString() const override {
    return "sizeof(" + type->toString() + ")";
  }
};

class BlockExpr final : public Expression {
public:
  std::vector<ExpressionPtr> body;

  BlockExpr(std::vector<ExpressionPtr> body, Range range)
      : Expression(ExpressionKind::ExprBlockKind, std::move(range)),
        body(std::move(body)) {}

  std::string toString() const override {
    return std::string("Block(" + std::to_string(body.size()) + " exprs)");
  }
};

class DeclExpr final : public Expression {
public:
  DeclarationPtr decl;

  DeclExpr(DeclarationPtr decl, Range range)
      : Expression(ExpressionKind::ExprDeclKind, std::move(range)),
        decl(std::move(decl)) {}

  std::string toString() const override {
    return "DeclExpr(" + decl->toString() + ")";
  }
};

class IfExpr final : public Expression {
public:
  ExpressionPtr condition;
  ExpressionPtr thenBranch;
  ExpressionPtr elseBranch;

  IfExpr(ExpressionPtr condition, ExpressionPtr thenBranch,
         ExpressionPtr elseBranch, Range range)
      : Expression(ExpressionKind::ExprIfKind, std::move(range)),
        condition(std::move(condition)), thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}

  std::string toString() const override {
    return "IfExpr(" + condition->toString() +
           (elseBranch ? ", with else)" : ")");
  }
};

class WhileExpr final : public Expression {
public:
  ExpressionPtr condition;
  ExpressionPtr body;

  WhileExpr(ExpressionPtr condition, ExpressionPtr body, Range range)
      : Expression(ExpressionKind::ExprWhileKind, std::move(range)),
        condition(std::move(condition)), body(std::move(body)) {}

  std::string toString() const override {
    return "WhileExpr(" + condition->toString() + ")";
  }
};

class DoWhileExpr final : public Expression {
public:
  ExpressionPtr body;
  ExpressionPtr condition;

  DoWhileExpr(ExpressionPtr body, ExpressionPtr condition, Range range)
      : Expression(ExpressionKind::ExprDoWhileKind, std::move(range)),
        body(std::move(body)), condition(std::move(condition)) {}

  std::string toString() const override {
    return "DoWhileExpr(" + condition->toString() + ")";
  }
};

class ForExpr final : public Expression {
public:
  ExpressionPtr init;
  ExpressionPtr condition;
  ExpressionPtr step;
  ExpressionPtr body;

  ForExpr(ExpressionPtr init, ExpressionPtr condition, ExpressionPtr step,
          ExpressionPtr body, Range range)
      : Expression(ExpressionKind::ExprForKind, std::move(range)),
        init(std::move(init)), condition(std::move(condition)),
        step(std::move(step)), body(std::move(body)) {}

  std::string toString() const override { return "ForExpr"; }
};

class ReturnExpr final : public Expression {
public:
  ExpressionPtr value;

  ReturnExpr(ExpressionPtr value, Range range)
      : Expression(ExpressionKind::ExprReturnKind, std::move(range)),
        value(std::move(value)) {}

  std::string toString() const override {
    return "ReturnExpr(" + (value ? value->toString() : "") + ")";
  }
};

class BreakExpr final : public Expression {
public:
  explicit BreakExpr(Range range)
      : Expression(ExpressionKind::ExprBreakKind, std::move(range)) {}

  std::string toString() const override { return "BreakExpr"; }
};

class ContinueExpr final : public Expression {
public:
  explicit ContinueExpr(Range range)
      : Expression(ExpressionKind::ExprContinueKind, std::move(range)) {}

  std::string toString() const override { return "ContinueExpr"; }
};
