#pragma once

#include "../../core/Range.hpp"

#include <memory>
#include <string>
#include <utility>

enum class TypeKind { TypeNamedKind, TypePointerKind, TypeArrayKind };

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
