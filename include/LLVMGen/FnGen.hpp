#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

namespace fngen {

class FunctionGen;

class Symbol{
  llvm::Value* value;
  llvm::Type* type;
  FunctionGen& parent;

  public:
  Symbol(llvm::Value* value, llvm::Type* type, FunctionGen& parent): value(value), type(type), parent(parent) {}
  Symbol(llvm::Value* value, FunctionGen& parent): value(value), type(value->getType()), parent(parent) {}
  Symbol(FunctionGen& parent): value(nullptr), type(nullptr), parent(parent) {}
  FunctionGen& getParent() { return parent; }
  llvm::Value* getValue() { return value; }
};

class Expr;
using ExprPtr = std::unique_ptr<Expr>;
class Expr : public Symbol {

public:
  Expr(llvm::Value* value, llvm::Type* type, FunctionGen& parent): Symbol(value, type, parent) {}
  Expr(llvm::Value* value, FunctionGen& parent): Symbol(value, parent) {}
  Expr(FunctionGen& parent): Symbol(parent) {}
  llvm::Value* eval();
};


enum ArithOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
};


class FunctionGen {
  llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>
      *builder;
  llvm::Function *fnPtr;
  llvm::BasicBlock *currentBB = nullptr;

public:
  FunctionGen(llvm::IRBuilder<llvm::ConstantFolder,
                              llvm::IRBuilderDefaultInserter> *builder,
              llvm::Function *fnPtr);

  Expr getArg(int i);
  void Return(Expr& expr);
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

}
