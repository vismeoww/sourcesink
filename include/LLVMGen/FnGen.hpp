#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"

#include <unordered_map>

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
  FunctionGen& getParent() const { return parent; }
  llvm::Value* getValue() const { return value; }
  llvm::BasicBlock* getBB() const { return bb; }
  llvm::Type* getType() const { return type; }
};


class Expr;
using ExprPtr = std::unique_ptr<Expr>;
class Expr : public Symbol {

public:
  Expr(llvm::Value* value, llvm::Type* type, FunctionGen& parent);
  Expr(llvm::Value* value, FunctionGen& parent);
  llvm::Value* eval() const {return getValue(); }
};

class Const;
class Variable : public Symbol {
  llvm::AllocaInst* address;
  public:
  Variable(llvm::AllocaInst* address, llvm::Type* type, FunctionGen& parent) : Symbol(nullptr, type, parent), address(address) {}
  llvm::AllocaInst* getAddress() { return address; }
  Expr load();
  // void store(Expr& expr);
  void store(Symbol& expr);
  void store(Expr& expr);
  void store(Const& expr);
  Variable& operator=(Symbol& expr) {
    store(expr);
    return *this;
  }
  Variable& operator=(Expr& expr) {
    store(expr);
    return *this;
  }
  Variable& operator=(Const& expr) {
    store(expr);
    return *this;
  }
};

class Const : public Expr {
public:
  Const(llvm::Value* value, llvm::Type* type, FunctionGen& parent): Expr(value,type,parent) {}
  Const(llvm::Value* value, FunctionGen& parent): Expr(value, parent) {}
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
  std::unordered_map<std::string, Variable> variables;

public:
  FunctionGen(llvm::IRBuilder<llvm::ConstantFolder,
                              llvm::IRBuilderDefaultInserter> *builder,
              llvm::Function *fnPtr);

  llvm::IRBuilder<>& getBuilder() { return *builder; }
  llvm::BasicBlock* getCurrentBB() { return currentBB; }
  Expr getArg(int i);
  void Return(const Expr& expr);
  void IfBlock(const Expr& cond);
  void ElseBlock();
  void ElseIfBlock(const Expr& cond);
  void EndIfBlock();
  void WhileBlock(const Expr& cond);
  void EndWhileBlock();
  void Break();
  void Continue();
  Expr createPhi(Expr& lhs, Expr& rhs);
  // for variables
  Variable& createVar(std::string name, llvm::Type* type);
  Variable& getVariable(std::string name);
  // for constants
  Const getInt(int64_t val, llvm::Type* type = nullptr);
  Const getTrue();
  Const getFalse();

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
