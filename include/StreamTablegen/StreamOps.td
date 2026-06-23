// include/Stream/StreamOps.td
#ifndef STREAM_OPS
#define STREAM_OPS

include "mlir/IR/OpBase.td"
include "StreamTablegen/StreamDialect.td"
include "StreamTablegen/StreamTypes.td"



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

def Stream_LatestEventToStateOp : Stream_Op<"latestEventToState", [
  // This forces the inner 'T' of $input and $result to match exactly
  AllTypesMatch<["input", "result"]>
]> {
  let summary = "Converts the latest event in a stream to a continuous state cell.";

  let arguments = (ins
    IsEventTypeOf:$input
  );

  let results = (outs
    IsStateTypeOf:$result
  );

  // Optional: Automatically maps the assembly to look clean
  let assemblyFormat = "$input attr-dict `:` type($input) `->` type($result)";
}


def Stream_StateChangeEventsOp : Stream_Op<"stateChangeEvents", [
  // This forces the inner 'T' of $input and $result to match exactly
  AllTypesMatch<["input", "result"]>
]> {
  let summary = "Whenever a state is changed, emit an event.";

  let arguments = (ins
    IsStateTypeOf:$input
  );

  let results = (outs
    IsEventTypeOf:$result
  );

  // Optional: Automatically maps the assembly to look clean
  let assemblyFormat = "$input attr-dict `:` type($input) `->` type($result)";
}

#endif // STREAM_OPS
