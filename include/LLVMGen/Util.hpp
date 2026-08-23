#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"


using namespace llvm;

BasicBlock* createForLoop(IRBuilder<>& builder, Function* func, StringRef name, BasicBlock* entry);
