#include "LLVMGen/FnGen.hpp"

using namespace fngen;
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
