#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {

void Codegen::createTidurFunction() {
    std::vector<llvm::Type*> sleepArgs;
    sleepArgs.push_back(getIntType());  // takes int seconds

    // Declare sleep function properly (returns int)
    llvm::FunctionType* sleepType = llvm::FunctionType::get(
        getIntType(),  // Return type (int)
        sleepArgs,     // Params
        false          // Not variadic
    );
    
    llvm::Function* sleepFunc = llvm::Function::Create(
        sleepType,
        llvm::Function::ExternalLinkage,
        "sleep",
        module.get()
    );

    // Create a wrapper function "tidur" (void tidur(int))
    llvm::FunctionType* tidurType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context),
        {getIntType()},  // (int seconds)
        false
    );
    
    llvm::Function* tidurFunc = llvm::Function::Create(
        tidurType,
        llvm::Function::ExternalLinkage,
        "tidur",
        module.get()
    );

    // Create the function body
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", tidurFunc);
    builder->SetInsertPoint(entry);

    // Get function arguments
    auto args = tidurFunc->arg_begin();
    llvm::Value* duration = args;

    // Call sleep (fix: assign return value)
    std::vector<llvm::Value*> sleepArgs2;
    sleepArgs2.push_back(duration);
    builder->CreateCall(sleepFunc, sleepArgs2);  // Ignore return value

    builder->CreateRetVoid();
    
    functions["tidur"] = tidurFunc;
}
}