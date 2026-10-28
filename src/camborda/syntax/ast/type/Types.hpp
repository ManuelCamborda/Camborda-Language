#pragma once

#include "../../../core/Range.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class TypeKind { TypeNamedKind, TypeGenericKind, TypeArrayKind };

class TypeNode {
public:
  TypeKind kind;
  Range range;

  TypeNode(TypeKind kind, Range range) : kind(kind), range(std::move(range)) {}
  virtual ~TypeNode() = default;

  bool is(TypeKind k) const { return kind == k; }

  template <typename T> const T *as() const {
    return dynamic_cast<const T *>(this);
  }
  template <typename T> bool isA() const { return as<T>() != nullptr; }

  virtual std::string toString() const = 0;
};

using TypePtr = std::unique_ptr<TypeNode>;

class NamedType final : public TypeNode {
public:
  std::string name;

  NamedType(std::string name, Range range)
      : TypeNode(TypeKind::TypeNamedKind, std::move(range)),
        name(std::move(name)) {}

  std::string toString() const override { return name; }
};

class GenericType final : public TypeNode {
public:
  std::string name;
  std::vector<TypePtr> args;

  GenericType(std::string name, std::vector<TypePtr> args, Range range)
      : TypeNode(TypeKind::TypeGenericKind, std::move(range)),
        name(std::move(name)), args(std::move(args)) {}

  std::string toString() const override {
    std::string out = name + "<";
    for (size_t i = 0; i < args.size(); i++) {
      if (i > 0)
        out += ", ";
      out += args[i]->toString();
    }
    return out + ">";
  }
};

class ArrayType final : public TypeNode {
public:
  TypePtr element;
  std::optional<uint64_t> size;

  ArrayType(TypePtr element, std::optional<uint64_t> size, Range range)
      : TypeNode(TypeKind::TypeArrayKind, std::move(range)),
        element(std::move(element)), size(size) {}

  std::string toString() const override {
    return "[" + element->toString() +
           (size ? "; " + std::to_string(*size) : "") + "]";
  }
};
