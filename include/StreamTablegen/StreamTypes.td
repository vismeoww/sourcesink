#ifndef STREAM_TYPES
#define STREAM_TYPES

include "mlir/IR/OpBase.td"
include "StreamTablegen/StreamDialect.td"


// Base class for Stream Types
class Stream_Type<string name, string typeMnemonic> : TypeDef<Stream_Dialect, name> {
  let mnemonic = typeMnemonic;
}

// 1. Event Type
def Stream_Event : Stream_Type<"Event", "event"> {
  let summary = "A discrete stream of events";
  let parameters = (ins "Type":$elementType);
  let assemblyFormat = "`<` $elementType `>`";
}

// 2. State Type
def Stream_State : Stream_Type<"State", "state"> {
  let summary = "A continuous state cell";
  let parameters = (ins "Type":$elementType);
  let assemblyFormat = "`<` $elementType `>`";
}

// 3. Optional Type
def Stream_Optional : Stream_Type<"Optional", "optional"> {
  let summary = "An optional type for values that may or may not be present";
  let parameters = (ins "Type":$elementType);
  let assemblyFormat = "`<` $elementType `>`";
}

// 4. Atomic Type 
def Stream_Atomic : Stream_Type<"Atomic", "atomic"> {
  let summary = "An atomic type for values that may or may not be present";
  let parameters = (ins "Type":$elementType);
  let assemblyFormat = "`<` $elementType `>`";
}

// Constraint: Validates that a type is a Stream_Event and extracts its element type
def IsEventTypeOf : TypeConstraint<
  // Explicitly write out your dialect's C++ namespace here
  CPred<"llvm::isa<::mlir::stream::EventType>($_self)">,
  "a stream event type"
> {
  // This allows AllTypesMatch to fetch the inner 'T'
  string transformer = "llvm::cast<::mlir::stream::EventType>($_self).getElementType()";
}

// Constraint: Validates that a type is a Stream_State and extracts its element type
def IsStateTypeOf : TypeConstraint<
  // Explicitly write out your dialect's C++ namespace here
  CPred<"llvm::isa<::mlir::stream::StateType>($_self)">,
  "a stream state type"
> {
  // This allows AllTypesMatch to fetch the inner 'T'
  string transformer = "llvm::cast<::mlir::stream::StateType>($_self).getElementType()";
}

def IsOptionalTypeOf : TypeConstraint<
  // Explicitly write out your dialect's C++ namespace here
  CPred<"llvm::isa<::mlir::stream::OptionalType>($_self)">,
  "a stream optional type"
> {
  // This allows AllTypesMatch to fetch the inner 'T'
  string transformer = "llvm::cast<::mlir::stream::OptionalType>($_self).getElementType()";
}

def IsAtomicTypeOf : TypeConstraint<
  // Explicitly write out your dialect's C++ namespace here
  CPred<"llvm::isa<::mlir::stream::AtomicType>($_self)">,
  "a stream atomic type"
> {
  // This allows AllTypesMatch to fetch the inner 'T'
  string transformer = "llvm::cast<::mlir::stream::AtomicType>($_self).getElementType()";
}


// Constraint: Type must be either a Stream_Event or Stream_State
def StreamEventOrState : TypeConstraint<
  Or<[Stream_Event.predicate, Stream_State.predicate]>,
  "stream event or state type"
>;

// Constraint: Enforces that the result type matches the operand type exactly
def AllSameStreamType : AllTypesMatch<["lhs", "rhs", "result"]>;

#endif // STREAM_TYPES
