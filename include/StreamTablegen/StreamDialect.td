#ifndef STREAM_DIALECT
#define STREAM_DIALECT

include "mlir/IR/OpBase.td"

// 1. Define the Dialect
def Stream_Dialect : Dialect {
    let name = "stream";
    let summary = "A custom Kafka-like stream processing dialect.";
    let cppNamespace = "::mlir::stream";
    let useDefaultTypePrinterParser = 1;
// Inject the declaration into the generated C++ class body
    let extraClassDeclaration = [{
        void registerTypes();
    }];
}

#endif // STREAM_DIALECT
