#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"

namespace fngen {

class FunctionGen;

class Symbol{
  llvm::Value* value;
  llvm::Type* type;
  FunctionGen& parent;
  llvm::BasicBlock* bb;

  public:
  Symbol(llvm::Value* value, llvm::Type* type, FunctionGen& parent, llvm::BasicBlock* bb): value(value), type(type), parent(parent), bb(bb) {}
  Symbol(llvm::Value* value, FunctionGen& parent, llvm::BasicBlock* bb): value(value), type(value->getType()), parent(parent), bb(bb) {}

  Symbol(llvm::Value* value, llvm::Type* type, FunctionGen& parent): value(value), type(type), parent(parent), bb(nullptr) {}
  Symbol(llvm::Value* value, FunctionGen& parent): value(value), type(value->getType()), parent(parent) {}
  Symbol(FunctionGen& parent, llvm::BasicBlock* bb): value(nullptr), type(nullptr), parent(parent), bb(bb) {}
  Symbol(FunctionGen& parent): value(nullptr), type(nullptr), parent(parent) {}
  FunctionGen& getParent() { return parent; }
  llvm::Value* getValue() { return value; }
  llvm::BasicBlock* getBB() { return bb; }
};

class Expr;
using ExprPtr = std::unique_ptr<Expr>;
class Expr : public Symbol {

public:
  Expr(llvm::Value* value, llvm::Type* type, FunctionGen& parent);//: Symbol(value, type, parent) {}
  Expr(llvm::Value* value, FunctionGen& parent);//: Symbol(value, parent) {}
  Expr(FunctionGen& parent);//: Symbol(parent) {}
  llvm::Value* eval();
};



struct BlockStackItem {
  llvm::BasicBlock* thenBB;
  llvm::BasicBlock* elseBB;
  llvm::BasicBlock* mergeBB;
  llvm::Instruction* condBr;
};

class FunctionGen {
  llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>
      *builder;
  llvm::SmallVector<BlockStackItem, 8> blockStack;
  llvm::Function *fnPtr;
  llvm::BasicBlock *currentBB = nullptr;

public:
  FunctionGen(llvm::IRBuilder<llvm::ConstantFolder,
                              llvm::IRBuilderDefaultInserter> *builder,
              llvm::Function *fnPtr);

  llvm::BasicBlock* getCurrentBB() { return currentBB; }
  Expr getArg(int i);
  void Return(Expr& expr);
  void IfBlock(Expr& cond);
  void ElseBlock();
  void ElseIfBlock(Expr& cond);
  void EndIfBlock();
  Expr createPhi(Expr& lhs, Expr& rhs);
  llvm::IRBuilder<>& getBuilder() { return *builder; }
};


// Operator Overloadings 

inline Expr operator+(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateAdd(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}


inline Expr operator-(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateSub(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator*(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateMul(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator/(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateSDiv(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator&&(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateAnd(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator||(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateOr(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator!(Symbol& lhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateNot(lhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator==(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpEQ(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator!=(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpNE(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator<(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpSLT(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator<=(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpSLE(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator>(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpSGT(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

inline Expr operator>=(Symbol& lhs, Symbol& rhs){
  fngen::FunctionGen& parent = lhs.getParent();
  llvm::Value* res = parent.getBuilder().CreateICmpSGE(lhs.getValue(), rhs.getValue());
  return fngen::Expr(res, parent);
}

}
