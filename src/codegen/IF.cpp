#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {


void Codegen::generateIf(const IfStmt* ifStmt, llvm::Function* currentFunction) {
    llvm::Value* condValue = generateExpr(ifStmt->condition.get());
    
    // Convert condition to bool
    condValue = builder->CreateICmpNE(
        condValue,
        llvm::ConstantInt::get(getIntType(), 0),
        "ifcond"
    );
    
    // Create blocks for the then case and merge
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "then", currentFunction);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont", currentFunction);
    
    builder->CreateCondBr(condValue, thenBB, mergeBB);
    
    // Emit then block
    builder->SetInsertPoint(thenBB);
    
    // Generate code for all statements in the then block
    for (const auto& stmt : ifStmt->thenBranch) {
        if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            generateReturn(ret.get(), currentFunction);
        }
        else if (auto var = std::dynamic_pointer_cast<VarDeclStmt>(stmt)) {
            generateVarDecl(var.get(), currentFunction);
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(stmt)) {
            generateExprStmt(exprStmt.get(), currentFunction);
        }
    }
    
    // Create branch to merge block if there's no terminator
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Set insertion point to merge block for further code
    builder->SetInsertPoint(mergeBB);
}

  
}