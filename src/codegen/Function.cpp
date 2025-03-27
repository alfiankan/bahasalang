#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {

void Codegen::generateFunction(const FunctionStmt* func) {
    llvm::Function* function = functions[func->name];
    if (!function) {
        llvm::report_fatal_error(llvm::Twine("Function not found: ") + func->name);
    }
    
    // Create entry block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);
    
    // Clear named values and add parameters
    namedValues.clear();
    for (auto& arg : function->args()) {
        namedValues[std::string(arg.getName())] = &arg;
    }
    
    // Generate function body
    for (const auto& stmt : func->body) {
        if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            generateReturn(ret.get(), function);
        }
        else if (auto var = std::dynamic_pointer_cast<VarDeclStmt>(stmt)) {
            generateVarDecl(var.get(), function);
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            generateIf(ifStmt.get(), function);
        }
        else if (auto tryStmt = std::dynamic_pointer_cast<TryStmt>(stmt)) {
            generateTryBlock(tryStmt.get(), function);
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(stmt)) {
            generateExprStmt(exprStmt.get(), function);
        }
    }
    
    // Verify function
    llvm::verifyFunction(*function);
}

void Codegen::generateReturn(const ReturnStmt* ret, llvm::Function* currentFunction) {
    llvm::Value* returnValue = generateExpr(ret->value.get());
    builder->CreateRet(returnValue);
}


llvm::Value* Codegen::generateCall(const CallExpr* call) {
    llvm::Function* callee = functions[call->callee];
    if (!callee) {
        llvm::report_fatal_error(llvm::Twine("Fungsi tidak dikenal: ") + call->callee);
    }
    
    std::vector<llvm::Value*> argsV;
    if (callee->getName() == "tampilkan") {
        if (call->arguments.size() < 2) {
            llvm::report_fatal_error("tampilkan membutuhkan minimal 2 argumen: string format dan nilai");
        }
        
        // First argument should be format string
        if (auto formatStr = std::dynamic_pointer_cast<StringExpr>(call->arguments[0])) {
            argsV.push_back(getStringConstant(formatStr->value));
        } else {
            llvm::report_fatal_error("Argumen pertama tampilkan harus berupa string format");
        }
        
        // Second argument should be string or value
        if (auto strArg = std::dynamic_pointer_cast<StringExpr>(call->arguments[1])) {
            argsV.push_back(getStringConstant(strArg->value));
        } else {
            argsV.push_back(generateExpr(call->arguments[1].get()));
        }
        
        builder->CreateCall(callee, argsV);
        return llvm::ConstantInt::get(getIntType(), 0); // Return dummy value
    }
    else if (callee->getName() == "tidur") {
        if (call->arguments.size() < 1) {
            llvm::report_fatal_error("tidur membutuhkan minimal 1 argumen: integer");
        }
        argsV.push_back(generateExpr(call->arguments[0].get()));
        builder->CreateCall(callee, argsV);
        return llvm::ConstantInt::get(getIntType(), 0); // Return dummy value
    }
    
    // Handle normal function calls
    for (const auto& arg : call->arguments) {
        argsV.push_back(generateExpr(arg.get()));
    }
    return builder->CreateCall(callee, argsV, "calltmp");
}
  
}