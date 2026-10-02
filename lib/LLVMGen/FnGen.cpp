#include "LLVMGen/FnGen.hpp"

fngen::Expr::Expr(llvm::Value *value, llvm::Type *type, FunctionGen &parent)
    : Symbol(value, type, parent, parent.getCurrentBB()) {}

fngen::Expr::Expr(llvm::Value *value, FunctionGen &parent)
    : Symbol(value, parent, parent.getCurrentBB()) {}

fngen::Expr::Expr(FunctionGen &parent) : Symbol(parent, parent.getCurrentBB()) {}

llvm::Value *fngen::Expr::eval() { return getValue(); }

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

void fngen::FunctionGen::Return(Expr &expr) { builder->CreateRet(expr.eval()); }

void fngen::FunctionGen::IfBlock(Expr &cond) {
  auto& Context = builder->GetInsertBlock()->getContext();

  auto *thenBB = llvm::BasicBlock::Create(Context, "then", fnPtr);
  auto *mergeBB = llvm::BasicBlock::Create(Context, "merge", fnPtr);

  auto *condBr = builder->CreateCondBr(cond.eval(), thenBB, mergeBB);
  builder->SetInsertPoint(thenBB);
  blockStack.push_back({thenBB, nullptr, mergeBB, condBr});
  currentBB = thenBB;
}

void fngen::FunctionGen::ElseBlock() {
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

void fngen::FunctionGen::EndIfBlock() {
  auto& Ctx = blockStack.back();
  blockStack.pop_back();
  auto& Context = builder->GetInsertBlock()->getContext();

  if(!builder->GetInsertBlock()->hasTerminator()){
    builder->CreateBr(Ctx.mergeBB);
  }

  builder->SetInsertPoint(Ctx.mergeBB);
}

fngen::Expr fngen::FunctionGen::createPhi(Expr& lhs, Expr& rhs) {
  assert((lhs.eval()->getType() == rhs.eval()->getType())&&"lhs and rhs must have the same type");
  auto& Context = builder->GetInsertBlock()->getContext();

  auto *phi = builder->CreatePHI(lhs.eval()->getType(), 2);
  phi->addIncoming(lhs.eval(), lhs.getBB());
  phi->addIncoming(rhs.eval(), rhs.getBB());
  return fngen::Expr(phi, *this);
}

