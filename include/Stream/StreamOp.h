#ifndef STREAM_STREAMOP_H
#define STREAM_STREAMOP_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

// Crucial: Include the TableGen-generated header declarations
#define GET_OP_CLASSES
#include "Stream/StreamOps.h.inc"

#endif // STREAM_STREAMOP_H
