#include "IR/MLIRGen.hpp"
#include "IR/TypeConv.hpp"
#include "llvm/ADT/SmallVector.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

MLIRGenImpl::MLIRGenImpl(mlir::MLIRContext &context) : builder(&context) {
  module = mlir::ModuleOp::create(builder.getUnknownLoc());
  this->dummyFileName = mlir::StringAttr::get(builder.getContext(), "dummy.mlir");
}

mlir::ModuleOp MLIRGenImpl::mlirgen(fn::FunctionPtr func) {
  builder.setInsertionPointToStart(module.getBody());
  this->mlirGen(std::move(func));
  return module;
}

mlir::ModuleOp MLIRGenImpl::mlirgen(module::ModulePtr moduleptr) {
  for (auto &fn : moduleptr->functions()) {
    builder.setInsertionPointToStart(module.getBody());
    this->mlirGen(std::move(fn));
  }
  return module;
}

mlir::func::FuncOp MLIRGenImpl::mlirGen(fn::FunctionPtr func) {
  llvm::SmallVector<mlir::Type, 4> argTypes;
  for (auto &arg : func->args()) {
    auto argType = getTypeFromString(builder, arg.second);
    argTypes.push_back(argType);
  }

  auto funcType = mlir::FunctionType::get(builder.getContext(), argTypes, {});

  auto function = mlir::func::FuncOp::create(builder, builder.getUnknownLoc(),
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
    if (stmt->type() == statement::Return) {
      auto stmtVal = processStmt(std::move(stmt));
      llvm::ArrayRef<mlir::Type> argTypes = function.getArgumentTypes();
      mlir::Type stmtType = stmtVal.getType();
      auto newFnType =
          mlir::FunctionType::get(builder.getContext(), argTypes, stmtType);
      function.setType(newFnType);
    } else {
      processStmt(std::move(stmt));
    }
  }
  return function;
}

mlir::Value MLIRGenImpl::processStmt(statement::StmtPtr stmt) {
  if (stmt->type() == statement::Assign) {
    auto *assignStmt = static_cast<statement::AssignStmt *>(stmt.get());
    auto lhs = assignStmt->lhs();
    auto &rhs = assignStmt->rhs();
    auto lhsVal = symbolTable[lhs];
    auto rhsVal = genExpr(std::move(rhs));
    symbolTable[lhs] = rhsVal;
    return rhsVal;
  } else if (stmt->type() == statement::Return) {
    // TODO: update function return type based on this
    auto *returnStmt = static_cast<statement::ReturnStmt *>(stmt.get());
    auto &expr = returnStmt->expr();
    mlir::Location loc = buildLoc(returnStmt->loc());
    auto exprVal = genExpr(std::move(expr));
    mlir::func::ReturnOp::create(builder, loc, exprVal);
    return exprVal;
  }
  assert(false && "unknown statement");
}

mlir::Value MLIRGenImpl::genExpr(expr::ExprPtr expr) {
  if (expr->type() == expr::kExprTInt) {
    return processIntExpr(static_cast<expr::IntValue *>(expr.get()));
  } else if (expr->type() == expr::kExprTFloat) {
    return processFloatExpr(static_cast<expr::FloatValue *>(expr.get()));
  } else if (expr->type() == expr::kExprTIdent) {
    return processIdentExpr(static_cast<expr::Ident *>(expr.get()));
  } else if (expr->type() == expr::kExprTBinop) {
    return processBinExpr(static_cast<expr::BinOp *>(expr.get()));
  } else if (expr->type() == expr::kExprTFuncCall) {
    return processFnCall(static_cast<expr::FunctionCall *>(expr.get()));
  }
  assert(false && "unknown expression");
}

mlir::Value MLIRGenImpl::processIntExpr(expr::IntValue *intExpr) {
  return mlir::arith::ConstantIntOp::create(builder, buildLoc(intExpr->loc()),
                                            intExpr->getValue(), 32);
}

mlir::Value MLIRGenImpl::processFloatExpr(expr::FloatValue *floatExpr) {
  mlir::FloatType floatType = builder.getF32Type();
  llvm::APFloat apFloat(floatExpr->getValue());
  return mlir::arith::ConstantFloatOp::create(builder, buildLoc(floatExpr->loc()),
                                              floatType, apFloat);
}

mlir::Value MLIRGenImpl::processIdentExpr(expr::Ident *identExpr) {
  return symbolTable[identExpr->getValue()];
}

mlir::Value MLIRGenImpl::processBinExpr(expr::BinOp *expr) {
  auto *binExpr = static_cast<expr::BinOp *>(expr);
  auto lhs = genExpr(std::move(binExpr->lhs()));
  auto rhs = genExpr(std::move(binExpr->rhs()));
  auto op = binExpr->op();
  // TODO: right now all types are assumed to be int
  if (op == expr::kBinOpAdd) {
    return mlir::arith::AddIOp::create(builder, buildLoc(binExpr->loc()), lhs,
                                       rhs);
  } else if (op == expr::kBinOpSub) {
    return mlir::arith::SubIOp::create(builder, buildLoc(binExpr->loc()), lhs,
                                       rhs);
  } else if (op == expr::kBinOpMul) {
    return mlir::arith::MulIOp::create(builder, buildLoc(binExpr->loc()), lhs,
                                       rhs);
  } else if (op == expr::kBinOpDiv) {
    return mlir::arith::DivSIOp::create(builder, buildLoc(binExpr->loc()), lhs,
                                        rhs);
  }
  assert(false && "unknown binary operator");
}

mlir::Value MLIRGenImpl::processFnCall(expr::FunctionCall *fnCall) {
  auto fn = fnCall->name();
  auto &args = fnCall->args();
  auto calleeFunc = module.lookupSymbol<mlir::func::FuncOp>(fn);
  if (!calleeFunc) {
    throw std::runtime_error("function not found: " + fn);
  }
  // 2. Evaluate all argument expressions to get their underlying mlir::Value
  // registers
  llvm::SmallVector<mlir::Value, 4> operands;
  for (auto &argExpr : args) {
    mlir::Value argValue = genExpr(std::move(argExpr));
    operands.push_back(argValue);
  }
  auto retType = calleeFunc.getResultTypes();
  auto callOp = mlir::func::CallOp::create(
      builder, buildLoc(fnCall->loc()), retType,
      mlir::SymbolRefAttr::get(builder.getContext(), fn), operands);
  return callOp.getResult(0);
}

mlir::Location MLIRGenImpl::buildLoc(Loc loc) {
  return mlir::FileLineColLoc::get(builder.getContext(), dummyFileName, loc.line, loc.column);
}
