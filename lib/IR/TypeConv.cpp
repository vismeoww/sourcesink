#include "IR/TypeConv.hpp"

#include "Stream/StreamType.h"

mlir::Type getTypeFromString(mlir::OpBuilder &builder, std::string typeName) {
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

mlir::Type getTypeFromPType(mlir::OpBuilder &builder, types::TypePtr &type) {
  if (type->kind() == types::TypeKind::Atomic) {
    return getTypeFromString(
        builder, static_cast<types::AtomicType *>(type.get())->name());
  } else if (type->kind() == types::TypeKind::ADTK) {
    std::string kind = static_cast<types::ADTKType *>(type.get())->name();
    if (kind == "Event") {
      assert(static_cast<types::ADTKType *>(type.get())->inner()->kind() ==
             types::TypeKind::Atomic);
      mlir::Type innerType = getTypeFromString(
          builder,
          static_cast<types::AtomicType *>(
              static_cast<types::ADTKType *>(type.get())->inner().get())
              ->name());
      return mlir::stream::EventType::get(builder.getContext(), innerType);
    } else if (kind == "State") {
      assert(static_cast<types::ADTKType *>(type.get())->inner()->kind() ==
             types::TypeKind::Atomic);
      mlir::Type innerType = getTypeFromString(
          builder,
          static_cast<types::AtomicType *>(
              static_cast<types::ADTKType *>(type.get())->inner().get())
              ->name());
      return mlir::stream::StateType::get(builder.getContext(), innerType);
    } else {
      assert(false && "unknown kind of type");
    }
    assert(false && "ADTK type not implemented");
  }
  assert(false && "unknown type");
}

bool isTypeEvent(const mlir::Type &type) {
  if (llvm::isa<mlir::stream::EventType>(type)) {
    return true;
  }
  return false;
}

bool isTypeState(const mlir::Type &type) {
  if (llvm::isa<mlir::stream::StateType>(type)) {
    return true;
  }
  return false;
}

bool isTypeOptional(const mlir::Type &type) {
  if (llvm::isa<mlir::stream::OptionalType>(type)) {
    return true;
  }
  return false;
}

bool isStreamType(const mlir::Type &type) {
  if (isTypeEvent(type) || isTypeState(type)) {
    return true;
  }
  return false;
}

template <typename T>
std::optional<mlir::Type> getInternalType(const mlir::Type &type) {
  if (llvm::isa<T>(type)) {
    return llvm::cast<T>(type).getElementType();
  }
  return std::nullopt;
}

StreamType streamType(const mlir::Type &type, bool shouldSimplify) {
  if (isTypeEvent(type)) {
    return kStreamEvent;
  } else if (isTypeState(type)) {
    return kStreamState;
  } else {
    if (shouldSimplify) {
      return KNonStream;
    } else {
      if (isTypeOptional(type)) {
        return kStreamOptional;
      } else {
        return kStreamAtomic;
      }
    }
  }
}

std::optional<mlir::Type> coersedType(const mlir::Type &lhs,
                                      const mlir::Type &rhs) {
  auto lhsType = streamType(lhs);
  auto rhsType = streamType(rhs);
  if (lhsType == rhsType) {
    return lhs;
  }
  if (lhsType == KNonStream && rhsType == kStreamEvent) {
    return rhs;
  }
  if (lhsType == KNonStream && rhsType == kStreamState) {
    return rhs;
  }
  if (lhsType == kStreamEvent && rhsType == KNonStream) {
    return lhs;
  }
  if (lhsType == kStreamState && rhsType == KNonStream) {
    return lhs;
  }
  return std::nullopt;
}

std::optional<mlir::Type> binaryOpResultType(const mlir::Type &lhs,
                                             const mlir::Type &rhs) {
  auto lhsType = streamType(lhs, false);
  auto rhsType = streamType(rhs, false);
  auto matchInternalType = [](const mlir::Type &lhs,
                              const mlir::Type &rhs) -> bool {
    if (streamType(lhs, false) == kStreamOptional ||
        streamType(rhs, false) == kStreamOptional) {
      return false;
    }
    if (streamType(lhs) == streamType(rhs)) {
      return true;
    }
    return false;
  };
  // TODO: This part is not correct, it's returning the internal type, but the caller 
  // is expecting the stream type
  if (lhsType == kStreamAtomic && rhsType == kStreamEvent) {
    // return compareInternals(
    //     lhs, getInternalType<mlir::stream::EventType>(rhs).value());
    if (matchInternalType(lhs, getInternalType<mlir::stream::EventType>(rhs).value())) {
      return rhs;
    }
  }
  if (lhsType == kStreamAtomic && rhsType == kStreamState) {
    // return compareInternals(
    //     lhs, getInternalType<mlir::stream::StateType>(rhs).value());
    if (matchInternalType(lhs, getInternalType<mlir::stream::StateType>(rhs).value())) {
      return rhs;
    }
  }
  if (lhsType == kStreamEvent && rhsType == kStreamAtomic) {
    // return compareInternals(
    //     getInternalType<mlir::stream::EventType>(lhs).value(), rhs);
    if (matchInternalType(getInternalType<mlir::stream::EventType>(lhs).value(), rhs)) {
      return lhs;
    }
  }
  if (lhsType == kStreamState && rhsType == kStreamAtomic) {
    // return compareInternals(
    //     getInternalType<mlir::stream::StateType>(lhs).value(), rhs);
    if (matchInternalType(getInternalType<mlir::stream::StateType>(lhs).value(), rhs)) {
      return lhs;
    }
  }
  // TODO: see what's being done here
  if (lhsType == kStreamEvent && rhsType == kStreamEvent) {
    if (matchInternalType(
            getInternalType<mlir::stream::EventType>(lhs).value(),
            getInternalType<mlir::stream::EventType>(rhs).value())) {
      return lhs;
    }
  }
  if (lhsType == kStreamState && rhsType == kStreamState) {
    if (matchInternalType(
            getInternalType<mlir::stream::StateType>(lhs).value(),
            getInternalType<mlir::stream::StateType>(rhs).value())) {
      return lhs;
    }
  }
  return std::nullopt;
}
