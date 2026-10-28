#pragma once

#include "../../../core/Range.hpp"

#include <memory>
#include <string>
#include <utility>

enum class DeclarationKind { DeclFunctionKind, DeclLetKind, DeclConstKind };

class Declaration {
public:
  DeclarationKind kind;
  Range range;

  Declaration(DeclarationKind kind, Range range)
      : kind(kind), range(std::move(range)) {}
  virtual ~Declaration() = default;

  bool is(DeclarationKind k) const { return kind == k; }

  template <typename T> const T *as() const {
    return dynamic_cast<const T *>(this);
  }
  template <typename T> bool isA() const { return as<T>() != nullptr; }

  virtual std::string toString() const = 0;
};

using DeclarationPtr = std::unique_ptr<Declaration>;
