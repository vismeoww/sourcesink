#include "Stream/StreamDialect.h"
#include "Stream/StreamType.h"

// LLVM Utilities
#include "llvm/ADT/TypeSwitch.h"

// MLIR Core Infrastructure
#include "mlir/IR/Builders.h"             
#include "mlir/IR/ImplicitLocOpBuilder.h" 
#include "mlir/IR/OpImplementation.h"     
#include "mlir/IR/BuiltinTypes.h"         
#include "mlir/IR/DialectImplementation.h"


#define GET_TYPEDEF_CLASSES
#include "Stream/StreamType.cpp.inc"

void mlir::stream::StreamDialect::registerTypes() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Stream/StreamType.cpp.inc"
  >();
}
