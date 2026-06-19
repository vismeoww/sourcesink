#pragma once
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Builders.h"
#include <string>
#include "parser/ast.hpp"

mlir::Type getTypeFromString(mlir::OpBuilder& builder, std::string typeName);

mlir::Type getTypeFromPType(mlir::OpBuilder& builder, types::TypePtr& type);
