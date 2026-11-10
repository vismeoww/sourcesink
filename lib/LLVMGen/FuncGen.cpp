#include "LLVMGen/FuncGen.hpp"

funcgen::FunctionGen::FunctionGen(
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

funcgen::ExprPtr funcgen::FunctionGen::getArg(int i) {
    return std::make_unique<funcgen::ValueExpr>(fnPtr->getArg(i), *this);
}

void funcgen::FunctionGen::Return(const ExprPtr &expr) { builder->CreateRet(expr->eval()); }

void funcgen::FunctionGen::IfBlock(const ExprPtr &cond) {
  auto& Context = builder->GetInsertBlock()->getContext();

  auto *thenBB = llvm::BasicBlock::Create(Context, "then", fnPtr);
  auto *mergeBB = llvm::BasicBlock::Create(Context, "merge", fnPtr);

  auto *condBr = builder->CreateCondBr(cond->eval(), thenBB, mergeBB);
  builder->SetInsertPoint(thenBB);
  blockStack.push_back({thenBB, nullptr, mergeBB, condBr});
  currentBB = thenBB;
}

void funcgen::FunctionGen::ElseBlock() {
  auto& Ctx = blockStack.back();
  auto& Context = builder->GetInsertBlock()->getContext();
  if(!builder->GetInsertBlock()->hasTerminator()){
    builder->CreateBr(Ctx.mergeBB);
  }
  Ctx.elseBB = llvm::BasicBlock::Create(Context, "else", fnPtr);
  Ctx.elseBB->moveBefore(Ctx.mergeBB);
  Ctx.condBr->setSuccessor(1, Ctx.elseBB);
  builder->SetInsertPoint(Ctx.elseBB);
  currentBB = Ctx.elseBB;
}

void funcgen::FunctionGen::ElseIfBlock(const ExprPtr& cond){
  auto& Ctx = blockStack.back();
  blockStack.pop_back();
  auto& Context = builder->GetInsertBlock()->getContext();
  builder->SetInsertPoint(Ctx.thenBB);
  if(!Ctx.thenBB->hasTerminator()){
    builder->CreateBr(Ctx.mergeBB);
  }
  auto *checkBB = llvm::BasicBlock::Create(Context, "elif-check", fnPtr);
  checkBB->moveBefore(Ctx.mergeBB);
  auto *elifThenBB = llvm::BasicBlock::Create(Context, "elif-then", fnPtr);
  elifThenBB->moveBefore(Ctx.mergeBB);
  Ctx.condBr->setSuccessor(1,checkBB);
  builder->SetInsertPoint(checkBB);
  currentBB = checkBB;
  auto *newCondBr = builder->CreateCondBr(cond->eval(),elifThenBB,Ctx.mergeBB);
  builder->SetInsertPoint(elifThenBB);
  blockStack.push_back({elifThenBB, nullptr, Ctx.mergeBB, newCondBr});
  currentBB = elifThenBB;
}

void funcgen::FunctionGen::EndIfBlock() {
  auto& Ctx = blockStack.back();
  blockStack.pop_back();
  auto& Context = builder->GetInsertBlock()->getContext();

  if(!builder->GetInsertBlock()->hasTerminator()){
    builder->CreateBr(Ctx.mergeBB);
  }

  builder->SetInsertPoint(Ctx.mergeBB);
}

funcgen::Variable& funcgen::FunctionGen::createVar(std::string name, llvm::Type* type) {
  auto* entry = builder->GetInsertBlock();
  auto* alloca = builder->CreateAlloca(type, nullptr, name);
  auto [it, inserted] = variables.try_emplace(name, alloca, type, *this);
  return it->second;
}
