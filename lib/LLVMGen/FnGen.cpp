#include "LLVMGen/FnGen.hpp"


llvm::Value* fngen::Expr::eval() {
  return getValue();
}

fngen::FunctionGen::FunctionGen(
    llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>
        *builder,
    llvm::Function *fnPtr) {
  this->builder = builder;
  this->fnPtr = fnPtr;
  // make sure there's no BasicBlocks inside the function 
  assert(fnPtr->empty() && "function already has basic blocks");
  // create a new basic block
  currentBB = llvm::BasicBlock::Create(builder->getContext(), "entry", fnPtr);
  builder->SetInsertPoint(currentBB);
}

fngen::Expr fngen::FunctionGen::getArg(int i) {
  return Expr(fnPtr->getArg(i), *this);
}

void fngen::FunctionGen::Return(Expr& expr) {
  builder->CreateRet(expr.eval());
}

