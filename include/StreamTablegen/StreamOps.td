// include/Stream/StreamOps.td
#ifndef STREAM_OPS
#define STREAM_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

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

// Binary operators

class Stream_BinaryOp<string mnemonic, list<Trait> traits = []> :
    Op<Stream_Dialect, mnemonic, !listconcat(traits, [AllSameStreamType, Pure])> {
      
  let summary = "Binary arithmetic operation on streams";
  
  // Both inputs must satisfy the event-or-state constraint
  let arguments = (ins
    StreamEventOrState:$lhs,
    StreamEventOrState:$rhs
  );

  // The output matches the input constraint
  let results = (outs
    StreamEventOrState:$result
  );

  // Simple assembly format: %result = stream.add %lhs, %rhs : !stream.event<i32>
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

def Stream_AddOp : Stream_BinaryOp<"add">;
def Stream_SubOp : Stream_BinaryOp<"sub">;
def Stream_MulOp : Stream_BinaryOp<"mul">;
def Stream_DivOp : Stream_BinaryOp<"div">;

#endif // STREAM_OPS
