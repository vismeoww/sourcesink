#include "JIT/jit.hpp"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "LLVMGen/Opt.hpp"

#include "LLVMGen/FnGen.hpp"

using namespace llvm;
using namespace llvm::orc;

// A helper function to handle LLVM Expected<T> errors cleanly
template <typename T> T checkError(Expected<T> Val) {
  if (!Val) {
    errs() << "LLVM Error: " << Val.takeError() << "\n";
    exit(1);
  }
  return std::move(*Val);
}

void checkError(Error Err) {
  if (Err) {
    errs() << "LLVM Error: " << Err << "\n";
    exit(1);
  }
}

int buildModule(std::unique_ptr<Module> &M,
                std::unique_ptr<llvm::LLVMContext> &Context) {
  IRBuilder<> Builder(*Context);
  // 4. Define the function signature: int add(int a, int b)
  Type *Int32Ty = Builder.getInt32Ty();
  FunctionType *FuncTy = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);
  Function *AddFunc =
      Function::Create(FuncTy, Function::ExternalLinkage, "add", M.get());

  // Name the arguments for clarity
  auto Args = AddFunc->arg_begin();
  Value *ArgA = &*Args++;
  ArgA->setName("a");
  Value *ArgB = &*Args;
  ArgB->setName("b");

  // 5. Build the function body
  BasicBlock *BB = BasicBlock::Create(*Context, "entry", AddFunc);
  Builder.SetInsertPoint(BB);

  // Perform the addition: a + b
  Value *Sum = Builder.CreateAdd(ArgA, ArgB, "addtmp");
  // Return the result
  Builder.CreateRet(Sum);
  return 0;
}

int buildModule2(std::unique_ptr<Module> &M,
                std::unique_ptr<llvm::LLVMContext> &Context) {
  IRBuilder<> Builder(*Context);
  Type *Int32Ty = Builder.getInt32Ty();
  FunctionType *FuncTy = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);
  Function *AddFunc =
      Function::Create(FuncTy, Function::ExternalLinkage, "fnfn", M.get());

  using namespace fngen;
  FunctionGen fgen(&Builder, AddFunc);
  Expr lhs = fgen.getArg(0);
  Expr rhs = fgen.getArg(1);
  Expr res = lhs + rhs;
  Expr cond = lhs > rhs;
  fgen.IfBlock(cond);
  Expr res2 = lhs * rhs;
  fgen.ElseBlock();
  Expr res3 = lhs - rhs;
  fgen.EndIfBlock();
  Expr res4 = fgen.createPhi(res2, res3);
  fgen.Return(res4);
  return 0;
}

int jitApp(RunMode mode) {
  // 1. Initialize the native target info for JIT execution
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  // 2. Create the LLJIT instance
  auto JIT = checkError(LLJITBuilder().create());
  auto Context = std::make_unique<LLVMContext>();

  // 3. Create an LLVM Module and an IR Builder
  auto M = std::make_unique<Module>("my_jit_module", *Context);
  buildModule2(M, Context);

  // (Optional) Print the generated IR to see what we made
  std::cout << "--- Generated LLVM IR ---" << std::endl;
  M->print(errs(), nullptr);
  std::cout << "-------------------------\n" << std::endl;
  if (mode == OnlyFnGen) {
    return 0;
  }

  // optimize the module
  optimizeModule(*M);

  std::cout << "--- Optimized LLVM IR ---" << std::endl;
  M->print(errs(), nullptr);
  std::cout << "-------------------------\n" << std::endl;

  if (mode == FnGenAndOpt) {
    return 0;
  }


  // 6. Hand over the module to the JIT compiler
  // ThreadSafeModule wraps the module and its context together safely
  checkError(
      JIT->addIRModule(ThreadSafeModule(std::move(M), std::move(Context))));

  // 7. Look up the compiled function's memory address
  auto AddSymbol = checkError(JIT->lookup("fnfn"));

  // Cast the raw address to a standard C++ function pointer
  auto *AddFnPtr = AddSymbol.toPtr<int (*)(int, int)>();

  // 8. Execute the dynamically generated function!
  int arg1 = 25;
  int arg2 = 15;
  int result = AddFnPtr(arg1, arg2);
  std::cout << "Result of dynamically calling add("<<arg1<<", "<<arg2<< "): " << result
            << std::endl;

  return 0;
}
