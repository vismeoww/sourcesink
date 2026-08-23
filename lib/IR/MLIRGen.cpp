#include "IR/MLIRGen.hpp"
#include "IR/TypeConv.hpp"
#include "llvm/ADT/SmallVector.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

/*
 * MLIRGenImpl
 * constructor
 */
MLIRGenImpl::MLIRGenImpl(mlir::MLIRContext &context, std::string moduleName)
    : builder(&context) {
  module = mlir::ModuleOp::create(builder.getUnknownLoc());
  this->dummyFileName = mlir::StringAttr::get(builder.getContext(), moduleName);
}

/*
 * Get all operators
 */
const bool MLIRGenImpl::isExistingOp(std::string fnname) {
  for (auto &op : allOps()) {
    if (fnname == op) {
      return true;
    }
  }
  return false;
}

/*
 * Generate MLIR for a function
 *
 * @param func: function to generate MLIR for
 * @return: MLIR module
 */
mlir::ModuleOp MLIRGenImpl::mlirgen(fn::FunctionPtr& func) {
  builder.setInsertionPointToStart(module.getBody());
  this->processFunction(func);
  return module;
}

/*
 * Generate MLIR for a module
 *
 * @param moduleptr: module to generate MLIR for
 * @return: MLIR module
 */
mlir::ModuleOp MLIRGenImpl::mlirgen(module::ModulePtr& moduleptr) {
  for (auto &fn : moduleptr->functions()) {
    builder.setInsertionPointToStart(module.getBody());
    this->processFunction(fn);
  }
  return module;
}

/*
 * Generate MLIR for a function
 *
 * @param func: function to generate MLIR for
 * @return: MLIR function
 */
mlir::func::FuncOp MLIRGenImpl::processFunction(fn::FunctionPtr& func) {
  // clear symbol table
  symbolTable.clear();
  // see if the function is an operator
  if (isExistingOp(func->name())) {
    std::string errorMsg =
        "function name cannot be an operator: " + func->name();
    throw std::runtime_error(errorMsg);
  }
  llvm::SmallVector<mlir::Type, 4> argTypes;
  for (auto &arg : func->args()) {
    // auto argType = getTypeFromString(builder, arg.second);
    auto argType = getTypeFromPType(builder, arg.second);
    argTypes.push_back(argType);
  }

  auto funcType = mlir::FunctionType::get(builder.getContext(), argTypes, {});

  auto function = mlir::func::FuncOp::create(builder, builder.getUnknownLoc(),
                                             func->name(), funcType);
  mlir::Block *entryBlock = function.addEntryBlock();
  builder.setInsertionPointToStart(entryBlock);

  for (const auto &pair : llvm::zip(func->args(), function.getArguments())) {
    auto &arg = std::get<0>(pair);
    auto argName = std::get<0>(arg);
    auto argVal = std::get<1>(pair);
    symbolTable[argName] = argVal;
  }

  for (auto &stmt : func->body()) {
    if (stmt->type() == statement::Return) {
      auto stmtVal = processStmt(stmt);
      llvm::ArrayRef<mlir::Type> argTypes = function.getArgumentTypes();
      mlir::Type stmtType = stmtVal.getType();
      auto newFnType =
          mlir::FunctionType::get(builder.getContext(), argTypes, stmtType);
      function.setType(newFnType);
    } else {
      processStmt(stmt);
    }
  }
  return function;
}

/*
 * Generate MLIR for a statement
 *
 * @param stmt: statement to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processStmt(statement::StmtPtr& stmt) {
  if (stmt->type() == statement::Assign) {
    auto *assignStmt = static_cast<statement::AssignStmt *>(stmt.get());
    auto lhs = assignStmt->lhs();
    auto &rhs = assignStmt->rhs();
    auto lhsVal = symbolTable[lhs];
    auto rhsVal = genExpr(rhs);
    symbolTable[lhs] = rhsVal;
    return rhsVal;
  } else if (stmt->type() == statement::Return) {
    // TODO: update function return type based on this
    auto *returnStmt = static_cast<statement::ReturnStmt *>(stmt.get());
    auto &expr = returnStmt->expr();
    mlir::Location loc = buildLoc(returnStmt->loc());
    auto exprVal = genExpr(expr);
    mlir::func::ReturnOp::create(builder, loc, exprVal);
    return exprVal;
  }
  assert(false && "unknown statement");
}

/*
 * Generate MLIR for an expression
 *
 * @param expr: expression to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::genExpr(expr::ExprPtr& expr) {
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

/*
 * Generate MLIR for an integer expression
 *
 * @param intExpr: integer expression to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processIntExpr(expr::IntValue *intExpr) {
  return mlir::arith::ConstantIntOp::create(builder, buildLoc(intExpr->loc()),
                                            intExpr->getValue(), 32);
}

/*
 * Generate MLIR for a float expression
 *
 * @param floatExpr: float expression to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processFloatExpr(expr::FloatValue *floatExpr) {
  mlir::FloatType floatType = builder.getF32Type();
  llvm::APFloat apFloat(floatExpr->getValue());
  return mlir::arith::ConstantFloatOp::create(
      builder, buildLoc(floatExpr->loc()), floatType, apFloat);
}

/*
 * Generate MLIR for an identifier expression
 *
 * @param identExpr: identifier expression to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processIdentExpr(expr::Ident *identExpr) {
  return symbolTable[identExpr->getValue()];
}

mlir::Value MLIRGenImpl::processStreamBinExpr(mlir::Value lhs, mlir::Value rhs,
                                              expr::BinOpType op, Loc loc,
                                              mlir::Type coersedType) {
  if (op == expr::kBinOpAdd) {
    return add(builder, lhs, rhs, buildLoc(loc), coersedType);
  } else if (op == expr::kBinOpSub) {
    return sub(builder, lhs, rhs, buildLoc(loc), coersedType);
  } else if (op == expr::kBinOpMul) {
    return mul(builder, lhs, rhs, buildLoc(loc), coersedType);
  } else if (op == expr::kBinOpDiv) {
    return div(builder, lhs, rhs, buildLoc(loc), coersedType);
  }
  assert(false && "unknown binary operator");
}
/*
 * Generate MLIR for a binary expression
 *
 * @param expr: binary expression to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processBinExpr(expr::BinOp *expr) {
  auto *binExpr = static_cast<expr::BinOp *>(expr);
  auto lhs = genExpr(binExpr->lhs());
  auto rhs = genExpr(binExpr->rhs());
  auto op = binExpr->op();
  if (isStreamType(lhs.getType()) || isStreamType(rhs.getType())) {
    auto coercedType = binaryOpResultType(lhs.getType(), rhs.getType());
    if (coercedType.has_value()) {
      return processStreamBinExpr(lhs, rhs, op, binExpr->loc(),
                                  coercedType.value());
    } else {
      std::string binExprStr = binExpr->toString();
      throw std::runtime_error(
          "processBinExpr: lhs and rhs cannot be coersed at : " +
          locToSting(binExpr->loc()) + "\n" + binExprStr);
    }
  }
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

/*
 * Generate MLIR for a function call
 *
 * @param fnCall: function call to generate MLIR for
 * @return: MLIR value
 */
mlir::Value MLIRGenImpl::processFnCall(expr::FunctionCall *fnCall) {

  auto fn = fnCall->name();
  auto &args = fnCall->args();
  int numArgs = args.size();
  // see if the function is an operator
  if (numArgs == 1) {
    if (unaryOpMap.find(fn) != unaryOpMap.end()) {
      auto op = unaryOpMap.at(fn);
      return op(builder, genExpr(args[0]));
    }
  } else if (numArgs == 2) {
    if (binaryOpMap.find(fn) != binaryOpMap.end()) {
      auto op = binaryOpMap.at(fn);
      return op(builder, genExpr(args[0]),
                genExpr(args[1]));
    }
  }
  // if not see if the function is defined in the module
  auto calleeFunc = module.lookupSymbol<mlir::func::FuncOp>(fn);
  if (!calleeFunc) {
    throw std::runtime_error("function not found: " + fn);
  }
  // 2. Evaluate all argument expressions to get their underlying mlir::Value
  // registers
  llvm::SmallVector<mlir::Value, 4> operands;
  for (auto &argExpr : args) {
    mlir::Value argValue = genExpr(argExpr);
    operands.push_back(argValue);
  }
  auto retType = calleeFunc.getResultTypes();
  auto callOp = mlir::func::CallOp::create(
      builder, buildLoc(fnCall->loc()), retType,
      mlir::SymbolRefAttr::get(builder.getContext(), fn), operands);
  return callOp.getResult(0);
}

/*
 * Helper function to build MLIR location
 *
 * @param loc: location to build location for
 * @return: MLIR location
 */
mlir::Location MLIRGenImpl::buildLoc(Loc loc) {
  return mlir::FileLineColLoc::get(builder.getContext(), dummyFileName,
                                   loc.line, loc.column);
}
