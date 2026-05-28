#include "parser/ast.hpp"


std::string expr::binOpTypeToString(BinOpType type) {
  switch (type) {
  case kBinOpAdd:
    return "Add";
  case kBinOpSub:
    return "Sub";
  case kBinOpMul:
    return "Mul";
  case kBinOpDiv:
    return "Div";
  case kBinOpEq:
    return "Eq";
  case kBinOpNEq:
    return "NEq";
  case kBinOpLT:
    return "LT";
  case kBinOpLE:
    return "LE";
  case kBinOpGT:
    return "GT";
  case kBinOpGE:
    return "GE";
  default:
    return "Unknown";
  }
}
