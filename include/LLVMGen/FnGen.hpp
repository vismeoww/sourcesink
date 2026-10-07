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

Expr operator+(Symbol& lhs, Symbol& rhs);
Expr operator-(Symbol& lhs, Symbol& rhs);
Expr operator*(Symbol& lhs, Symbol& rhs);
Expr operator/(Symbol& lhs, Symbol& rhs);
Expr operator&&(Symbol& lhs, Symbol& rhs);
Expr operator||(Symbol& lhs, Symbol& rhs);
Expr operator!(Symbol& lhs);
Expr operator==(Symbol& lhs, Symbol& rhs);
Expr operator!=(Symbol& lhs, Symbol& rhs);
Expr operator<(Symbol& lhs, Symbol& rhs);
Expr operator<=(Symbol& lhs, Symbol& rhs);
Expr operator>(Symbol& lhs, Symbol& rhs);
Expr operator>=(Symbol& lhs, Symbol& rhs);

}
