#include "Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include "std/PrintFunction.cpp"
#include "std/SleepFunction.cpp"
#include "Function.cpp"
#include "BinaryOp.cpp"
#include "FixedArray.cpp"
#include "IF.cpp"
#include "Try.cpp"
#include "VariableDecl.cpp"

namespace bahasa {

Codegen::Codegen(std::string moduleName) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    
}


void Codegen::generate(const std::vector<StmtPtr>& statements) {
    // First pass: Forward declare all functions and check if tampilkan is used
    bool needsPrintFunction = false;
    bool needsTidurFunction = false;
    
    // First check if we need tampilkan
    for (const auto& stmt : statements) {
        if (auto func = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
            for (const auto& bodyStmt : func->body) {
                if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(bodyStmt)) {
                    // Check inside if statements
                    for (const auto& thenStmt : ifStmt->thenBranch) {
                        if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(thenStmt)) {
                            if (auto call = std::dynamic_pointer_cast<CallExpr>(exprStmt->expr)) {
                                if (call->callee == "tampilkan") {
                                    needsPrintFunction = true;
                                }
                                if (call->callee == "tidur") {
                                    needsTidurFunction = true;
                                }
                            }
                        }
                    }
                }
                else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(bodyStmt)) {
                    if (auto call = std::dynamic_pointer_cast<CallExpr>(exprStmt->expr)) {
                        if (call->callee == "tampilkan") {
                            needsPrintFunction = true;
                        }
                        if (call->callee == "tidur") {
                            needsTidurFunction = true;
                        }
                    }
                }
            }
        }
    }
    
    // Create needed functions
    if (needsPrintFunction) {
        createPrintFunction();
    }

    if (needsTidurFunction) {
        createTidurFunction();
    }
    
    // Forward declare all user functions
    for (const auto& stmt : statements) {
        if (auto func = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
            std::vector<llvm::Type*> paramTypes(func->params.size(), getIntType());
            
            llvm::FunctionType* funcType = llvm::FunctionType::get(
                getIntType(),
                paramTypes,
                false
            );
            
            llvm::Function* function = llvm::Function::Create(
                funcType,
                llvm::Function::ExternalLinkage,
                func->name,
                module.get()
            );
            
            // Set parameter names
            unsigned idx = 0;
            for (auto& param : function->args()) {
                param.setName(func->params[idx++].name);
            }
            
            functions[func->name] = function;
        }
    }
    
    // Generate function bodies
    for (const auto& stmt : statements) {
        if (auto func = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
            generateFunction(func.get());
        }
    }

}


void Codegen::generateExprStmt(const ExprStmt* stmt, llvm::Function* currentFunction) {
    generateExpr(stmt->expr.get());
}

llvm::Value* Codegen::generateExpr(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return generateNumber(num);
    }
    else if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        return generateVariable(var);
    }
    else if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        return generateBinary(binary);
    }
    else if (auto comp = dynamic_cast<const ComparisonExpr*>(expr)) {
        return generateComparison(comp);
    }
    else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        return generateCall(call);
    }
    else if (auto assign = dynamic_cast<const AssignmentExpr*>(expr)) {
        return generateAssignment(assign);
    }
    else if (auto arrayLit = dynamic_cast<const ArrayLiteralExpr*>(expr)) {
        return generateArrayLiteral(arrayLit);
    }
    else if (auto arrayIndex = dynamic_cast<const ArrayIndexExpr*>(expr)) {
        return generateArrayIndex(arrayIndex, nullptr);
    }
    
    llvm::report_fatal_error(llvm::Twine("Tipe ekspresi tidak dikenal"));
    return nullptr;
}

llvm::Value* Codegen::generateNumber(const NumberExpr* num) {
    return llvm::ConstantInt::get(getIntType(), num->value);
}


llvm::Value* Codegen::getStringConstant(const std::string& str) {
    return builder->CreateGlobalStringPtr(str);
}

llvm::Type* Codegen::getIntType() {
    return llvm::Type::getInt32Ty(*context);
}

void Codegen::dump(llvm::raw_ostream& os) const {
    module->print(os, nullptr);
}

} // namespace bahasa 