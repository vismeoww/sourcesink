#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace expr {

enum ExprType {
  kExprTInt,
  kExprTFloat,
  kExprTString,
  kExprTBool,
  kExprTIdent,
  kExprTBinop,
  KExprTGroup,
  kExprTFuncCall,
};

// base class for all expressions
class Expr {
public:
  virtual ~Expr() = default;
  virtual ExprType type() const = 0;
  virtual std::string toString() const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

// Integer literal
class IntValue final : public Expr {
  int value_;

public:
  IntValue(int value) : value_(value) {}
  virtual ~IntValue() = default;
  ExprType type() const override { return kExprTInt; }
  std::string toString() const override {
    return "IntValue(" + std::to_string(value_) + ")";
  }
  int getValue() const { return value_; }
};

// Float literal
class FloatValue final : public Expr {
  double value_;

public:
  FloatValue(double value) : value_(value) {}
  virtual ~FloatValue() = default;
  ExprType type() const override { return kExprTFloat; }
  std::string toString() const override {
    return "FloatValue(" + std::to_string(value_) + ")";
  }
  float getValue() const { return value_; }
};

// boolean literal
class BoolValue final : public Expr {
  bool value_;

public:
  BoolValue(bool value) : value_(value) {}
  virtual ~BoolValue() = default;
  ExprType type() const override { return kExprTBool; }
  std::string toString() const override {
    return "BoolValue(" + std::to_string(value_) + ")";
  }
};

// string literal
class StringValue final : public Expr {
  std::string value_;

public:
  StringValue(std::string value) : value_(value) {}
  virtual ~StringValue() = default;
  ExprType type() const override { return kExprTString; }
  std::string toString() const override {
    return "StringValue(" + value_ + ")";
  }
};

class Ident final : public Expr {
  std::string value_;

public:
  Ident(std::string value) : value_(value) {}
  virtual ~Ident() = default;
  ExprType type() const override { return kExprTIdent; }
  std::string toString() const override {
    return "Ident(" + value_ + ")";
  }
  std::string getValue() const { return value_; }
};

enum BinOpType {
  kBinOpAdd,
  kBinOpSub,
  kBinOpMul,
  kBinOpDiv,
  kBinOpEq,
  kBinOpNEq,
  kBinOpLT,
  kBinOpLE,
  kBinOpGT,
  kBinOpGE,
};

std::string binOpTypeToString(BinOpType type);

class BinOp final : public Expr {
  BinOpType op_;
  ExprPtr lhs_;
  ExprPtr rhs_;

public:
  BinOp(BinOpType op, ExprPtr lhs, ExprPtr rhs)
      : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
  virtual ~BinOp() = default;
  ExprType type() const override { return kExprTBinop; }
  ExprPtr& lhs()  { return lhs_; }
  ExprPtr& rhs()  { return rhs_; }
  BinOpType op() const { return op_; }
  std::string toString() const override {
    return "BinOp(" + binOpTypeToString(op_) + ", " + lhs_->toString() + ", " + rhs_->toString() + ")";
  }
};

class Group final : public Expr {
  ExprPtr expr_;

public:
  Group(ExprPtr expr) : expr_(std::move(expr)) {}
  virtual ~Group() = default;
  ExprType type() const override { return KExprTGroup; }
  ExprPtr& expr()  { return expr_; }
  std::string toString() const override {
    return "Group(" + expr_->toString() + ")";
  }
};

class FunctionCall final : public Expr {
  std::string name_;
  std::vector<ExprPtr> args_;

public:
  FunctionCall(std::string name, std::vector<ExprPtr> args)
      : name_(name), args_(std::move(args)) {}
  virtual ~FunctionCall() = default;
  ExprType type() const override { return kExprTFuncCall; }
  std::string name() const { return name_; }
  std::vector<ExprPtr>& args()  { return args_; }
  std::string toString() const override {
    std::string argsStr;
    for (auto& arg : args_) {
      argsStr += arg->toString() + ", ";
    }
    return "FunctionCall(" + name_ + "(" + argsStr + "))";
  }
};

} // namespace expr

namespace statement {

enum StmtType {
  Assign,
  Return,
};

class Statement {
  public:
    virtual ~Statement() = default;
    virtual StmtType type() const = 0;
    virtual std::string toString() const = 0;
};

using StmtPtr = std::unique_ptr<Statement>;

class AssignStmt final : public Statement {
  std::string lhs_;
  expr::ExprPtr rhs_;

public:
  AssignStmt(std::string lhs, expr::ExprPtr rhs)
      : lhs_(lhs), rhs_(std::move(rhs)) {}
  virtual ~AssignStmt() = default;
  StmtType type() const override { return Assign; }
  std::string lhs() const { return lhs_; }
  expr::ExprPtr& rhs()  { return rhs_; }
  std::string toString() const override {
    return "AssignStmt(" + lhs_ + ", " + rhs_->toString() + ")";
  }
};

class ReturnStmt final : public Statement {
  expr::ExprPtr expr_;

public:
  ReturnStmt(expr::ExprPtr expr) : expr_(std::move(expr)) {}
  virtual ~ReturnStmt() = default;
  StmtType type() const override { return Return; }
  expr::ExprPtr& expr()  { return expr_; }
  std::string toString() const override {
    return "ReturnStmt(" + expr_->toString() + ")";
  }
};


} // namespace statement

namespace fn {
  class Function {
    std::string name_;
    std::vector<std::pair<std::string, std::string>> args_;
    std::vector<statement::StmtPtr> body_;
    public:
      Function(std::string name, std::vector<std::pair<std::string, std::string>> args, std::vector<statement::StmtPtr> body)
          : name_(name), args_(std::move(args)), body_(std::move(body)) {}
      virtual ~Function() = default;
      std::string name() const { return name_; }
      std::vector<std::pair<std::string, std::string>>& args()  { return args_; }
      std::vector<statement::StmtPtr>& body()  { return body_; }
      std::string toString() const {
        std::string argsStr;
        for (auto& arg : args_) {
          argsStr += arg.first + ":" + arg.second + ", ";
        }
        std::string bodyStr;
        for (auto& stmt : body_) {
          bodyStr += stmt->toString() + ", ";
        }
        return "Function(" + name_ + "(" + argsStr + ")){" + bodyStr + "}";
      }
  };
  using FunctionPtr = std::unique_ptr<Function>;
}

namespace module {
  class Module {
    std::vector<fn::FunctionPtr> functions_;
    public:
      Module(std::vector<fn::FunctionPtr> functions) : functions_(std::move(functions)) {}
      virtual ~Module() = default;
      std::vector<fn::FunctionPtr>& functions()  { return functions_; }
      std::string toString() const {
        std::string functionsStr;
        for (auto& fn : functions_) {
          functionsStr += fn->toString() + ", ";
        }
        return "Module(" + functionsStr + ")";
      }
  };
  using ModulePtr = std::unique_ptr<Module>;
}
