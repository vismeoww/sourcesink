#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

class Symbol{
  llvm::Value* value;
  llvm::Type* type;
};

class FunctionGen {
  llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>
      *builder;
  llvm::Function *fnPtr;

  FunctionGen(llvm::IRBuilder<llvm::ConstantFolder,
                              llvm::IRBuilderDefaultInserter> *builder,
              llvm::Function *fnPtr);
};
