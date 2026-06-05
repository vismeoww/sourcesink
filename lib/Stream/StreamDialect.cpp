#include "Stream/StreamDialect.h"

#include "mlir/IR/Builders.h"             // Fixes incomplete mlir::Builder / mlir::OpBuilder
#include "mlir/IR/ImplicitLocOpBuilder.h" // Fixes incomplete mlir::ImplicitLocOpBuilder
#include "mlir/IR/OpImplementation.h"     // Fixes incomplete mlir::OpAsmParser / OpAsmPrinter
#include "mlir/IR/BuiltinTypes.h"         // Fixes incomplete type logic like ::mlir::NoneType

// Crucial: Include the TableGen-generated implementation details
#include "Stream/StreamDialect.cpp.inc"

#define GET_OP_CLASSES
#include "Stream/StreamOps.cpp.inc"

void mlir::stream::StreamDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Stream/StreamOps.cpp.inc"
  >();
}
