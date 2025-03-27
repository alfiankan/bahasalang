#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {



void Codegen::generateTryBlock(const TryStmt* tryStmt, llvm::Function* currentFunction) {
    // Create blocks for try, error handling, and continue
    llvm::BasicBlock* tryBlock = llvm::BasicBlock::Create(*context, "try", currentFunction);
    llvm::BasicBlock* errorBlock = llvm::BasicBlock::Create(*context, "error", currentFunction);
    llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(*context, "try_continue", currentFunction);
    
    // Branch to try block
    builder->CreateBr(tryBlock);
    
    // Generate try block
    builder->SetInsertPoint(tryBlock);
    for (const auto& stmt : tryStmt->tryBlock) {
        if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            generateReturn(ret.get(), currentFunction);
        }
        else if (auto var = std::dynamic_pointer_cast<VarDeclStmt>(stmt)) {
            generateVarDecl(var.get(), currentFunction);
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            generateIf(ifStmt.get(), currentFunction);
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(stmt)) {
            if (auto arrayIndex = std::dynamic_pointer_cast<ArrayIndexExpr>(exprStmt->expr)) {
                generateArrayIndex(arrayIndex.get(), errorBlock);
            } else {
                generateExprStmt(exprStmt.get(), currentFunction);
            }
        }
    }
    
    // If we get here, no error occurred, branch to continue block
    builder->CreateBr(continueBlock);
    
    // Generate error block (returns 0 and continues)
    builder->SetInsertPoint(errorBlock);
    builder->CreateBr(continueBlock);
    
    // Set insertion point to continue block for further code
    builder->SetInsertPoint(continueBlock);
}

}