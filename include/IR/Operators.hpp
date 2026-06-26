#pragma once

#include <string>
#include <map>
#include <functional>

#include "mlir/IR/Dialect.h"

using UnaryOpFn = std::function<mlir::Value(mlir::OpBuilder&,mlir::Value)>;

mlir::Value latestEventToState(mlir::OpBuilder& builder, mlir::Value lastEvent);
mlir::Value stateChangeEvents(mlir::OpBuilder& builder, mlir::Value state);

const std::map<std::string, UnaryOpFn> unaryOpMap = {
  {"latestEventToState", latestEventToState},
  {"stateChangeEvents", stateChangeEvents},
};

mlir::Value add(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type);
mlir::Value sub(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type);
mlir::Value mul(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type);
mlir::Value div(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type);

typedef std::function<mlir::Value(mlir::OpBuilder&,mlir::Value, mlir::Value)> BinaryOpFn;
const std::map<std::string, BinaryOpFn> binaryOpMap = {};


const std::set<std::string> allOps();
