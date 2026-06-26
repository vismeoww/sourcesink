#include "Stream/StreamType.h"
#include "Stream/StreamOp.h"

#include "IR/Operators.hpp"
#include "mlir/IR/Builders.h"

mlir::Value latestEventToState(mlir::OpBuilder& builder, mlir::Value lastEvent) {
  // see if lastEvent is a stream event
  if(!llvm::isa<mlir::stream::EventType>(lastEvent.getType())){
    std::string ss;
    llvm::raw_string_ostream llvmss(ss);
    mlir::Type lastEventTypeName = lastEvent.getType();
    lastEventTypeName.print(llvmss);
    throw std::runtime_error("latestEventToState: lastEvent is not a stream event: "+ss);
  }
  auto eventType = llvm::cast<mlir::stream::EventType>(lastEvent.getType());
  mlir::Type elementType = eventType.getElementType();
  auto stateResultType = mlir::stream::StateType::get(builder.getContext(), elementType);
  auto op = mlir::stream::LatestEventToStateOp::create(builder, builder.getUnknownLoc(), stateResultType, lastEvent);
  return op->getResult(0);
}

mlir::Value stateChangeEvents(mlir::OpBuilder& builder, mlir::Value state) {
  // see if the state is a stream state
  if (!llvm::isa<mlir::stream::StateType>(state.getType())) {
    std::string ss;
    llvm::raw_string_ostream llvmss(ss);
    mlir::Type stateTypeName = state.getType();
    stateTypeName.print(llvmss);
    throw std::runtime_error("stateChangeEvents: state is not a stream state: "+ss);
  }
  auto stateType = llvm::cast<mlir::stream::StateType>(state.getType());
  mlir::Type elementType = stateType.getElementType();
  auto eventResultType = mlir::stream::EventType::get(builder.getContext(), elementType);
  auto op = mlir::stream::StateChangeEventsOp::create(builder, builder.getUnknownLoc(), eventResultType, state);
  return op->getResult(0);
}

mlir::Value add(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type) {
  auto op = mlir::stream::AddOp::create(builder, loc, type, lhs, rhs);
  return op->getResult(0);
}

mlir::Value sub(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc,mlir::Type type) {
  auto op = mlir::stream::SubOp::create(builder, loc, type, lhs, rhs);
  return op->getResult(0);
}

mlir::Value mul(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type) {
  auto op = mlir::stream::MulOp::create(builder, loc, type, lhs, rhs);
  return op->getResult(0);
}

mlir::Value div(mlir::OpBuilder& builder, mlir::Value lhs, mlir::Value rhs, mlir::Location loc, mlir::Type type) {
  auto op = mlir::stream::DivOp::create(builder, loc, type, lhs, rhs);
  return op->getResult(0);
}

const std::set<std::string> allOps(){
  static std::set<std::string> ops = {};
  for (auto& op : unaryOpMap) {
    ops.insert(op.first);
  }
  for (auto& op : binaryOpMap) {
    ops.insert(op.first);
  }
  return ops;
}
