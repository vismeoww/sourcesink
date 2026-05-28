#pragma once
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Builders.h"
#include <string>

mlir::Type getTypeFromString(mlir::OpBuilder& builder, std::string typeName);
