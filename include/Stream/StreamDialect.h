#ifndef STREAM_STREAMDIALECT_H
#define STREAM_STREAMDIALECT_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"

// Crucial: Include the TableGen-generated header declarations
#define GET_DIALECT_CLASSES
#include "Stream/StreamDialect.h.inc"


#define GET_OP_CLASSES
#include "Stream/StreamOps.h.inc"

#endif // STREAM_STREAMDIALECT_H
