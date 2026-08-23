#include "LLVMGen/Gen.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"


using namespace llvm;

LLVMGen::LLVMGen(llvm::LLVMContext* context) {
  this->context = context;
  this->module = std::make_unique<llvm::Module>("my_module", *context);
}

LLVMGen::~LLVMGen() {
}

std::unique_ptr<llvm::Module> LLVMGen::getModule() {
  return std::move(module);
}

int LLVMGen::generate(mlir::ModuleOp& mlirModule) {

  for (auto fn : mlirModule.getOps<mlir::func::FuncOp>()) {
    if (fn.getName() == "main") {
      Function* f = createMainFunction(fn);
    } else {
      Function* f = createFunction(fn, fn.getName());
    }
  }

  return 0;
}

Function* LLVMGen::createFunction(mlir::func::FuncOp& func, llvm::StringRef name) {
  llvm::IRBuilder<> builder(*context);
  // 4. Define the function signature: int add(int a, int b)
  Type *Int32Ty = builder.getInt32Ty();
  FunctionType *FuncTy = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);
  Function *AddFunc =
      Function::Create(FuncTy, Function::ExternalLinkage, name, module.get());

  // Name the arguments for clarity
  auto Args = AddFunc->arg_begin();
  Value *ArgA = &*Args++;
  ArgA->setName("a");
  Value *ArgB = &*Args;
  ArgB->setName("b");

  // 5. Build the function body
  BasicBlock *BB = BasicBlock::Create(*context, "entry", AddFunc);
  builder.SetInsertPoint(BB);

  // Perform the addition: a + b
  Value *Sum = builder.CreateAdd(ArgA, ArgB, "addtmp");
  // Return the result
  builder.CreateRet(Sum);
  return AddFunc;
}

Function* LLVMGen::createMainFunction(mlir::func::FuncOp& func) {
  llvm::IRBuilder<> builder(*context);

  // function signature
  Type *ptrCharTy = builder.getPtrTy();
  Type *ulongTy = builder.getInt64Ty();
  FunctionType *FuncTy = FunctionType::get(ulongTy, {ptrCharTy,ulongTy}, false);
  Function *mainFunc = Function::Create(FuncTy, Function::ExternalLinkage, "main", module.get());
  // arguments 
  auto Args = mainFunc->arg_begin();
  Value* strPtr = &*Args++;
  strPtr->setName("strPtr");
  Value* len = &*Args;
  len->setName("len");

  // entry block
  BasicBlock *BB = BasicBlock::Create(*context, "entry", mainFunc);
  builder.SetInsertPoint(BB);

  AllocaInst *i = builder.CreateAlloca(ulongTy, nullptr, "i");
  AllocaInst *count = builder.CreateAlloca(ulongTy, nullptr, "count");
  builder.CreateStore(ConstantInt::get(ulongTy, 0), i);
  builder.CreateStore(ConstantInt::get(ulongTy, 0), count);

  // for loop header
  BasicBlock *forHeader = BasicBlock::Create(*context, "for.loop.header", mainFunc);
  builder.CreateBr(forHeader);
  builder.SetInsertPoint(forHeader);
  Value* cmp = builder.CreateICmpSLT(builder.CreateLoad(ulongTy,i), len);

  BasicBlock *forBody = BasicBlock::Create(*context, "for.loop.body", mainFunc);
  BasicBlock *forEnd = BasicBlock::Create(*context, "for.loop.end", mainFunc);
  builder.CreateCondBr(cmp, forBody, forEnd);

  // for loop body
  builder.SetInsertPoint(forBody);
  Value* load = builder.CreateLoad(ptrCharTy,strPtr);
  Value* inc = builder.CreateAdd(builder.CreateLoad(ulongTy,i), ConstantInt::get(ulongTy, 1));
  builder.CreateStore(inc, i);
  builder.CreateStore(load, builder.CreateGEP(ptrCharTy,strPtr,inc));
  Value* cmp2 = builder.CreateICmpSLT(builder.CreateLoad(ulongTy,i), len);
  // builder.CreateCondBr(cmp2, forHeader, forEnd);
  builder.CreateBr(forHeader);

  // for loop end
  builder.SetInsertPoint(forEnd);


  // setup return value 
  Value* retVal = builder.CreateLoad(ulongTy,i);
  builder.CreateRet(retVal);
  return mainFunc;
}




