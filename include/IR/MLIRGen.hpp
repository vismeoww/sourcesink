#pragma once 
#include "parser/ast.hpp"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"

class MLIRGenImpl {
  public:
    MLIRGenImpl(mlir::MLIRContext& context) : builder(&context) {}
    mlir::ModuleOp mlirgen(fn::FunctionPtr func);
    void mlirGen(fn::FunctionPtr func);
    mlir::Value genExpr(expr::ExprPtr expr);
    void processStmt(statement::StmtPtr stmt);
  private:
    mlir::OpBuilder builder;
    mlir::ModuleOp module;
    std::map<std::string, mlir::Value> symbolTable;
};
