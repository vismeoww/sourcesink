#include "IR/TypeConv.hpp"

mlir::Type getTypeFromString(mlir::OpBuilder& builder, std::string typeName) {
  if (typeName == "i1") {
    return builder.getIntegerType(1);
  } else if (typeName == "i8") {
    return builder.getIntegerType(8);
  } else if (typeName == "i16") {
    return builder.getIntegerType(16);
  } else if (typeName == "i32") {
    return builder.getIntegerType(32);
  } else if (typeName == "i64") {
    return builder.getIntegerType(64);
  } else if (typeName == "f16") {
    return builder.getF16Type();
  } else if (typeName == "f32") {
    return builder.getF32Type();
  } else if (typeName == "f64") {
    return builder.getF64Type();
  } else if (typeName == "bf16") {
    return builder.getBF16Type();
  } else if (typeName == "index") {
    return builder.getIndexType();
  } else if (typeName == "none") {
    return builder.getNoneType();
  } else {
    assert(false && "unknown type");
  }
}

mlir::Type getTypeFromPType(mlir::OpBuilder& builder, types::TypePtr& type) {
  if (type->kind() == types::TypeKind::Atomic) {
    return getTypeFromString(builder, static_cast<types::AtomicType*>(type.get())->name());
  } else if (type->kind() == types::TypeKind::ADTK) {
    assert(false && "ADTK type not implemented");
  }
  assert(false && "unknown type");
}
