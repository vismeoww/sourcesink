#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"


#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"




class LLVMGen {
  public:
    LLVMGen(llvm::LLVMContext* context);
    ~LLVMGen();
    std::unique_ptr<llvm::Module> getModule();
    int generate(mlir::ModuleOp& mlirModule);

  private:
      std::unique_ptr<llvm::Module> module;
      llvm::LLVMContext* context;
      // std::unique_ptr<llvm::IRBuilder<>> builder;
      // builder functions 
      llvm::Function* createFunction(mlir::func::FuncOp& func, llvm::StringRef name);
      llvm::Function* createMainFunction(mlir::func::FuncOp& func);
};
