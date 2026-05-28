#include "IR/MLIRGen.hpp"
#include "IR/TypeConv.hpp"
#include "llvm/ADT/SmallVector.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

mlir::ModuleOp MLIRGenImpl::mlirgen(fn::FunctionPtr func) {
  auto theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
  builder.setInsertionPointToStart(theModule.getBody());
  this->mlirGen(std::move(func));
  return theModule;
}

void MLIRGenImpl::mlirGen(fn::FunctionPtr func) {
  llvm::SmallVector<mlir::Type, 4> argTypes;
  for (auto &arg : func->args()) {
    auto argType = getTypeFromString(builder, arg.second);
    argTypes.push_back(argType);
  }

  auto funcType = mlir::FunctionType::get(builder.getContext(), argTypes,
                                          builder.getIntegerType(32));

  auto function = builder.create<mlir::func::FuncOp>(builder.getUnknownLoc(),
                                                     func->name(), funcType);

  mlir::Block *entryBlock = function.addEntryBlock();
  builder.setInsertionPointToStart(entryBlock);

  for (const auto &pair : llvm::zip(func->args(), function.getArguments())) {
    auto arg = std::get<0>(pair);
    auto argName = std::get<0>(arg);
    auto argVal = std::get<1>(pair);
    symbolTable[argName] = argVal;
  }

  for (auto &stmt : func->body()) {
    processStmt(std::move(stmt));
  }
}

void MLIRGenImpl::processStmt(statement::StmtPtr stmt) {
  if (stmt->type() == statement::Assign) {
    auto *assignStmt = static_cast<statement::AssignStmt *>(stmt.get());
    auto lhs = assignStmt->lhs();
    auto &rhs = assignStmt->rhs();
    auto lhsVal = symbolTable[lhs];
    auto rhsVal = genExpr(std::move(rhs));
    symbolTable[lhs] = rhsVal;
    return;
  } else if (stmt->type() == statement::Return) {
    auto *returnStmt = static_cast<statement::ReturnStmt *>(stmt.get());
    auto &expr = returnStmt->expr();
    auto exprVal = genExpr(std::move(expr));
    mlir::func::ReturnOp::create(builder, builder.getUnknownLoc(), exprVal);
    return;
  }
  assert(false && "unknown statement");
}

mlir::Value MLIRGenImpl::genExpr(expr::ExprPtr expr) {
  if (expr->type() == expr::kExprTInt) {
    auto *intExpr = static_cast<expr::IntValue *>(expr.get());
    auto intType = builder.getIntegerType(32);
    auto apInt = builder.getIntegerAttr(intType, intExpr->getValue());
    return mlir::arith::ConstantIntOp::create(builder, builder.getUnknownLoc(),
                                              intExpr->getValue(), 32);
  } else if (expr->type() == expr::kExprTFloat) {
    auto *floatExpr = static_cast<expr::FloatValue *>(expr.get());
    mlir::FloatType floatType = builder.getF32Type();
    llvm::APFloat apFloat(floatExpr->getValue());
    return mlir::arith::ConstantFloatOp::create(
        builder, builder.getUnknownLoc(), floatType, apFloat);
  } else if (expr->type() == expr::kExprTIdent) {
    auto *identExpr = static_cast<expr::Ident *>(expr.get());
    return symbolTable[identExpr->getValue()];
  } else if (expr->type() == expr::kExprTBinop) {
    auto *binExpr = static_cast<expr::BinOp *>(expr.get());
    auto lhs = genExpr(std::move(binExpr->lhs()));
    auto rhs = genExpr(std::move(binExpr->rhs()));
    auto op = binExpr->op();
    // TODO: right now all types are assumed to be int
    if (op == expr::kBinOpAdd) {
      return mlir::arith::AddIOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    } else if (op == expr::kBinOpSub) {
      return mlir::arith::SubIOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    } else if (op == expr::kBinOpMul) {
      return mlir::arith::MulIOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    } else if (op == expr::kBinOpDiv) {
      return mlir::arith::DivSIOp::create(builder, builder.getUnknownLoc(), lhs, rhs);
    }
  }
  assert(false && "unknown expression");
}
