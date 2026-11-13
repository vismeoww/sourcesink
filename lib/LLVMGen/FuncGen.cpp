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
  blockStack.push_back(IfBlockItem{thenBB, nullptr, mergeBB, condBr});
  currentBB = thenBB;
}

void funcgen::FunctionGen::ElseBlock() {
  assert(!blockStack.empty() && "Else called when the block stack is empty");
  //auto IfItem = blockStack.back();
  auto* Ctx = std::get_if<IfBlockItem>(&blockStack.back());
  assert(Ctx && "Cannot match else block with non-if blockstack item");
  auto& Context = builder->GetInsertBlock()->getContext();
  if(!builder->GetInsertBlock()->hasTerminator()){
    builder->CreateBr(Ctx->mergeBB);
  }
  Ctx->elseBB = llvm::BasicBlock::Create(Context, "else", fnPtr);
  Ctx->elseBB->moveBefore(Ctx->mergeBB);
  Ctx->condBr->setSuccessor(1, Ctx->elseBB);
  builder->SetInsertPoint(Ctx->elseBB);
  currentBB = Ctx->elseBB;
}

void funcgen::FunctionGen::ElseIfBlock(const ExprPtr& cond){
  assert(!blockStack.empty() && "ElseIf called when the block stack is empty");
  auto* Ctx = std::get_if<IfBlockItem>(&blockStack.back());
  assert(Ctx && "Cannot match else-if block with non-if blockstack item");
  blockStack.pop_back();
  auto& Context = builder->GetInsertBlock()->getContext();
  builder->SetInsertPoint(Ctx->thenBB);
  if(!Ctx->thenBB->hasTerminator()){
    builder->CreateBr(Ctx->mergeBB);
  }
  auto *checkBB = llvm::BasicBlock::Create(Context, "elif-check", fnPtr);
  checkBB->moveBefore(Ctx->mergeBB);
  auto *elifThenBB = llvm::BasicBlock::Create(Context, "elif-then", fnPtr);
  elifThenBB->moveBefore(Ctx->mergeBB);
  Ctx->condBr->setSuccessor(1,checkBB);
  builder->SetInsertPoint(checkBB);
  currentBB = checkBB;
  auto *newCondBr = builder->CreateCondBr(cond->eval(),elifThenBB,Ctx->mergeBB);
  builder->SetInsertPoint(elifThenBB);
  blockStack.push_back(IfBlockItem{elifThenBB, nullptr, Ctx->mergeBB, newCondBr});
  currentBB = elifThenBB;
}

void funcgen::FunctionGen::EndIfBlock() {
  assert(!blockStack.empty() && "EndIf called when the block stack is empty");
  auto* Ctx = std::get_if<IfBlockItem>(&blockStack.back());
  assert(Ctx && "Cannot match endif block with non-if blockstack item");
  blockStack.pop_back();
  auto& Context = builder->GetInsertBlock()->getContext();

  if(!builder->GetInsertBlock()->hasTerminator()){
    builder->CreateBr(Ctx->mergeBB);
  }

  builder->SetInsertPoint(Ctx->mergeBB);
}

void funcgen::FunctionGen::WhileBlock(const ExprPtr &cond) {
  auto& Context = builder->GetInsertBlock()->getContext();

  auto *condBB = llvm::BasicBlock::Create(Context, "while.cond", fnPtr);
  auto *bodyBB = llvm::BasicBlock::Create(Context, "while.body", fnPtr);
  auto *afterBB = llvm::BasicBlock::Create(Context, "while.end", fnPtr);

  builder->CreateBr(condBB);
  builder->SetInsertPoint(condBB);

  // Evaluate condition inside condBB so it re-evaluates on every iteration
  builder->CreateCondBr(cond->eval(), bodyBB, afterBB);

  builder->SetInsertPoint(bodyBB);
  blockStack.push_back(WhileBlockItem{condBB, bodyBB, afterBB});
  currentBB = bodyBB;
}

void funcgen::FunctionGen::EndWhileBlock() {
  assert(!blockStack.empty() && "EndWhileBlock() called with empty block stack");

  auto* whileItem = std::get_if<WhileBlockItem>(&blockStack.back());
  assert(whileItem && "EndWhileBlock() called when top of stack is not a while block");

  // 1. Only emit the back-edge branch to condBB if the current block
  //    hasn't already been terminated (e.g., via Return(), Break(), or Continue()).
  llvm::BasicBlock* insertBB = builder->GetInsertBlock();
  if (insertBB && !insertBB->hasTerminator()) {
    builder->CreateBr(whileItem->condBB);
  }

  // 2. Move the insertion point to afterBB for subsequent instructions
  llvm::BasicBlock* afterBB = whileItem->afterBB;
  builder->SetInsertPoint(afterBB);
  currentBB = afterBB;

  // 3. Pop the loop frame off the stack
  blockStack.pop_back();
}

funcgen::Variable& funcgen::FunctionGen::createVar(std::string name, llvm::Type* type) {
  auto* alloca = builder->CreateAlloca(type, nullptr, name);
  auto [it, inserted] = variables.try_emplace(name, alloca, type, *this);
  return it->second;
}

funcgen::Variable& funcgen::FunctionGen::createVar(std::string name, const ExprPtr& e) {
  llvm::Type* t = e->getType();
  auto* alloca = builder->CreateAlloca(t, nullptr, name);
  auto [it, inserted] = variables.try_emplace(name, alloca, t, *this);
  it->second.store(e);
  return it->second;
}

const funcgen::ExprPtr funcgen::Variable::load() {
    return std::make_unique<LoadExpr>(address, getType(), getParent());
}

void funcgen::Variable::store(const ExprPtr& expr) {
    llvm::Value* val = expr->eval();
    parent.getBuilder().CreateStore(val, address);
}