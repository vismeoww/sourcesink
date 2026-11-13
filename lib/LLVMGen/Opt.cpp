#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/IR/Module.h"

#include "LLVMGen/Opt.hpp"

#include <cassert>

void optimizeModule(llvm::Module &M, int optlevel) {
    // 1. Create Analysis Managers
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // 2. Initialize the PassBuilder
    llvm::PassBuilder PB;

    // 3. Register all analysis passes with the respective analysis managers
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // 4. Build the standard optimization pipeline (e.g., OptimizationLevel::O2)
    auto optLevel = llvm::OptimizationLevel::O0;
    switch(optlevel){
        case 0: optLevel = llvm::OptimizationLevel::O0; break;
        case 1: optLevel = llvm::OptimizationLevel::O1; break;
        case 2: optLevel = llvm::OptimizationLevel::O2; break;
        case 3: optLevel = llvm::OptimizationLevel::O3; break;
        default: assert(0 && "Optimization level should be between 0 and 3");
    }
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(optLevel);

    // 5. Run the optimizations on your module!
    MPM.run(M, MAM);
}
