#pragma once 
#include "parser/ast.hpp"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "IR/Operators.hpp"

class MLIRGenImpl {
  public:
    MLIRGenImpl(mlir::MLIRContext& context, std::string moduleName);
    mlir::ModuleOp mlirgen(fn::FunctionPtr func);
    mlir::ModuleOp mlirgen(module::ModulePtr module);
    mlir::func::FuncOp processFunction(fn::FunctionPtr func);
    mlir::Value processStmt(statement::StmtPtr stmt);
    mlir::Value genExpr(expr::ExprPtr expr);
    // processing each expr 
    mlir::Value processIntExpr(expr::IntValue* intExpr);
    mlir::Value processFloatExpr(expr::FloatValue* floatExpr);
    mlir::Value processIdentExpr(expr::Ident* identExpr);
    mlir::Value processBinExpr(expr::BinOp* binExpr);
    mlir::Value processFnCall(expr::FunctionCall* fnCall);

    mlir::Location buildLoc(Loc loc);
  private:
    mlir::OpBuilder builder;
    mlir::ModuleOp module;
    std::map<std::string, mlir::Value> symbolTable;
    mlir::StringAttr dummyFileName;
    static const bool isExistingOp(std::string fnname);
};
