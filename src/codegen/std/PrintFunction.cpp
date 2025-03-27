#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {

void Codegen::createPrintFunction() {
    // First declare printf
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(llvm::Type::getInt8Ty(*context)->getPointerTo());  // char* format
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        builder->getInt32Ty(),     // Return type
        printfArgs,                // Params
        true                       // varargs
    );
    
    llvm::Function* printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );

    // Create tampilkan wrapper for printf
    llvm::FunctionType* tampilkanType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context),
        {llvm::Type::getInt8Ty(*context)->getPointerTo(), getIntType()},  // (char*, i32)
        false
    );
    
    llvm::Function* tampilkanFunc = llvm::Function::Create(
        tampilkanType,
        llvm::Function::ExternalLinkage,
        "tampilkan",
        module.get()
    );

    // Create the function body
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", tampilkanFunc);
    builder->SetInsertPoint(entry);

    // Get function arguments
    auto args = tampilkanFunc->arg_begin();
    llvm::Value* formatStr = args++;
    llvm::Value* value = args;

    // Call printf
    std::vector<llvm::Value*> printfArgs2;
    printfArgs2.push_back(formatStr);
    printfArgs2.push_back(value);
    builder->CreateCall(printfFunc, printfArgs2);
    
    builder->CreateRetVoid();
    
    functions["tampilkan"] = tampilkanFunc;
}
}