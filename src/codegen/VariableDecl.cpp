#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {


llvm::Value* Codegen::generateVariable(const VariableExpr* var) {
    llvm::Value* value = namedValues[var->name];
    if (!value) {
        llvm::report_fatal_error(llvm::Twine("Nama variabel tidak dikenal: ") + var->name);
    }
    
    // If it's an alloca instruction, load the value
    if (auto alloca = llvm::dyn_cast<llvm::AllocaInst>(value)) {
        return builder->CreateLoad(getIntType(), alloca, var->name + "_load");
    }
    
    return value;
}

void Codegen::generateVarDecl(const VarDeclStmt* var, llvm::Function* currentFunction) {
    llvm::Value* value = generateExpr(var->initializer.get());
    
    // If this is an array declaration
    if (value->getType()->isPointerTy()) {
        namedValues[var->name] = value; // Store the array pointer directly
        return;
    }
    
    // Create an alloca instruction in the entry block of the function
    llvm::IRBuilder<> tempBuilder(&currentFunction->getEntryBlock(), 
                                 currentFunction->getEntryBlock().begin());
    
    // Create alloca for the variable
    llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(getIntType(), nullptr, var->name);
    
    // Store the initial value
    builder->CreateStore(value, alloca);
    
    // Remember the alloca instruction for this variable
    namedValues[var->name] = alloca;
}

llvm::Value* Codegen::generateAssignment(const AssignmentExpr* assign) {
    // Get the variable's alloca instruction
    llvm::Value* variable = namedValues[assign->name];
    if (!variable) {
        llvm::report_fatal_error(llvm::Twine("Variabel tidak ditemukan: ") + assign->name);
    }
    
    // Generate the value to assign
    llvm::Value* value = generateExpr(assign->value.get());
    
    // Store the value
    builder->CreateStore(value, variable);
    
    // Return the assigned value
    return value;
}
  
}