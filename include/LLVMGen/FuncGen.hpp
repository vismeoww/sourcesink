#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"

#include <unordered_map>

namespace funcgen {

enum class OpKind {
  ADD, SUB, MUL, SDIV, UDIV,
  AND, OR, NOT,
  EQ, NE, SLT, SLE, SGT, SGE
};

class Expr;
using ExprPtr = std::shared_ptr<Expr>;

class FunctionGen;

class Expr {
protected:
  FunctionGen& parent;
  mutable llvm::Value* cachedValue = nullptr;

public:
  Expr(FunctionGen& parent) : parent(parent) {}
  virtual ~Expr() = default;

  FunctionGen& getParent() const { return parent; }

  // Materialize or fetch cached LLVM IR
  virtual llvm::Value* eval() const = 0;

  llvm::Value* getValue() const {
    if (!cachedValue) {
      cachedValue = eval();
    }
    return cachedValue;
  }
};


// Leaf node wrapping an existing llvm::Value* (e.g. function args, already loaded values)
class ValueExpr : public Expr {
  llvm::Value* val;
public:
  ValueExpr(llvm::Value* val, FunctionGen& parent)
      : Expr(parent), val(val) {}

  llvm::Value* eval() const override {
    return val;
  }
};

struct BlockStackItem {
  llvm::BasicBlock* thenBB;
  llvm::BasicBlock* elseBB;
  llvm::BasicBlock* mergeBB;
  llvm::Instruction* condBr;
};

class Variable;
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
  ExprPtr getArg(int i);
  void Return(const ExprPtr& expr);
  void IfBlock(const ExprPtr& cond);
  void ElseBlock();
  void ElseIfBlock(const ExprPtr& cond);
  void EndIfBlock();
  void WhileBlock(const ExprPtr& cond);
  void EndWhileBlock();
  void Break();
  void Continue();
  // ExprPtr createPhi(ExprPtr& lhs, ExprPtr& rhs);
  // for variables
  Variable& createVar(std::string name, llvm::Type* type);
  Variable& getVariable(std::string name);
};


// Binary Expression Node
class BinExpr : public Expr {
  ExprPtr lhs;
  ExprPtr rhs;
  OpKind op;

public:
  BinExpr(ExprPtr lhs, ExprPtr rhs, OpKind op, FunctionGen& parent)
      : Expr(parent), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

  llvm::Value* eval() const override {
    llvm::Value* l = lhs->getValue();
    llvm::Value* r = rhs->getValue();
    auto& builder = parent.getBuilder();

    switch (op) {
      case OpKind::ADD:  return builder.CreateAdd(l, r, "addtmp");
      case OpKind::SUB:  return builder.CreateSub(l, r, "subtmp");
      case OpKind::MUL:  return builder.CreateMul(l, r, "multmp");
      case OpKind::SDIV: return builder.CreateSDiv(l, r, "divtmp");
      case OpKind::AND:  return builder.CreateAnd(l, r, "andtmp");
      case OpKind::OR:   return builder.CreateOr(l, r, "ortmp");
      case OpKind::EQ:   return builder.CreateICmpEQ(l, r, "eqtmp");
      case OpKind::NE:   return builder.CreateICmpNE(l, r, "netmp");
      case OpKind::SLT:  return builder.CreateICmpSLT(l, r, "slttmp");
      case OpKind::SLE:  return builder.CreateICmpSLE(l, r, "sletmp");
      case OpKind::SGT:  return builder.CreateICmpSGT(l, r, "sgttmp");
      case OpKind::SGE:  return builder.CreateICmpSGE(l, r, "sgetmp");
      default:
        llvm_unreachable("Unsupported binary operator");
    }
  }
};

// Unary Expression Node (e.g. operator!)
class UnaryExpr : public Expr {
  ExprPtr operand;
  OpKind op;

public:
  UnaryExpr(ExprPtr operand, OpKind op, FunctionGen& parent)
      : Expr(parent), operand(std::move(operand)), op(op) {}

  llvm::Value* eval() const override {
    llvm::Value* v = operand->getValue();
    auto& builder = parent.getBuilder();

    switch (op) {
      case OpKind::NOT: return builder.CreateNot(v, "nottmp");
      default: llvm_unreachable("Unsupported unary operator");
    }
  }
};

class LoadExpr : public Expr {
  llvm::AllocaInst* address;
  llvm::Type* elemType;

public:
  LoadExpr(llvm::AllocaInst* addr, llvm::Type* type, FunctionGen& parent)
      : Expr(parent), address(addr), elemType(type) {}

  llvm::Value* eval() const override {
    return parent.getBuilder().CreateLoad(elemType, address, "loadtmp");
  }
};

class Variable {
  llvm::AllocaInst* address;
  llvm::Type* type;
  FunctionGen& parent;
  public:
  Variable(llvm::AllocaInst* address, llvm::Type* type, FunctionGen& parent): 
    parent(parent), type(type), address(address) {}
  llvm::AllocaInst* getAddress() { return address; }
  Expr load();
  // void store(Expr& expr);
  void store(Expr& expr);
  Variable& operator=(Expr& expr) {
    store(expr);
    return *this;
  }
};


inline ExprPtr operator+(const ExprPtr& lhs, const ExprPtr& rhs) {
  return std::make_shared<BinExpr>(lhs, rhs, OpKind::ADD, lhs->getParent());
}

inline ExprPtr operator==(const ExprPtr& lhs, const ExprPtr& rhs) {
  return std::make_shared<BinExpr>(lhs, rhs, OpKind::EQ, lhs->getParent());
}

inline ExprPtr operator!(const ExprPtr& operand) {
  return std::make_shared<UnaryExpr>(operand, OpKind::NOT, operand->getParent());
}

}