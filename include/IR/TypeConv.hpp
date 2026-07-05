#pragma once
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Builders.h"
#include <string>
#include "parser/ast.hpp"

enum StreamType {
  kStreamEvent,
  kStreamState,
  kStreamAtomic,
  kStreamOptional,
  KNonStream,
};

mlir::Type getTypeFromString(mlir::OpBuilder& builder, std::string typeName);

mlir::Type getTypeFromPType(mlir::OpBuilder& builder, types::TypePtr& type);

bool isTypeEvent(const mlir::Type& type);

bool isTypeState(const mlir::Type& type);

bool isTypeOptional(const mlir::Type& type);

bool isStreamType(const mlir::Type& type);


template<typename T>
std::optional<mlir::Type> getInternalType(const mlir::Type& type);

StreamType streamType(const mlir::Type& type, bool shouldSimplify = true);

std::optional<mlir::Type> coersedType(const mlir::Type& lhs, const mlir::Type& rhs);

std::optional<mlir::Type> binaryOpResultType(const mlir::Type& lhs, const mlir::Type& rhs);

