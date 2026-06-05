// include/Stream/StreamOps.td
#ifndef STREAM_OPS
#define STREAM_OPS

include "mlir/IR/OpBase.td"

// 1. Define the Dialect
def Stream_Dialect : Dialect {
    let name = "stream";
    let summary = "A custom Kafka-like stream processing dialect.";
    let cppNamespace = "::mlir::stream";
}

// 2. Define a Base Operation
class Stream_Op<string mnemonic, list<Trait> traits = []> :
    Op<Stream_Dialect, mnemonic, traits>;

// 3. Define a concrete operation (e.g., Emit a payload to a topic)
def Stream_EmitOp : Stream_Op<"emit"> {
    let summary = "Emit data to a specified stream topic.";
    let arguments = (ins AnyType:$data, StrAttr:$topic);
    let results = (outs);
    let assemblyFormat = "$data `to` $topic attr-dict `:` type($data)";
}

#endif // STREAM_OPS
