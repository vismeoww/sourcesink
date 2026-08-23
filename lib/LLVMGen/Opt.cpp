#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/IR/Module.h"

#include "LLVMGen/Opt.hpp"

void optimizeModule(llvm::Module &M) {
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
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    // 5. Run the optimizations on your module!
    MPM.run(M, MAM);
}
