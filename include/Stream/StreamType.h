#ifndef STREAM_STREMTYPE_H
#define STREAM_STREMTYPE_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"

// Crucial: Include the TableGen-generated header declarations
#define GET_TYPEDEF_CLASSES
#include "Stream/StreamType.h.inc"

#endif // STREAM_STREMTYPE_H
