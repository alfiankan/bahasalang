#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {


llvm::Value* Codegen::generateBinary(const BinaryExpr* binary) {
    llvm::Value* left = generateExpr(binary->left.get());
    llvm::Value* right = generateExpr(binary->right.get());
    
    if (binary->op == "+") {
        return builder->CreateAdd(left, right, "addtmp");
    }
    else if (binary->op == "-") {
        return builder->CreateSub(left, right, "subtmp");
    }
    else if (binary->op == "*") {
        return builder->CreateMul(left, right, "multmp");
    }
    else if (binary->op == "/") {
        return builder->CreateSDiv(left, right, "divtmp");
    }
    else if (binary->op == "modulo") {
        return builder->CreateSRem(left, right, "modtmp");
    }
    else if (binary->op == "dan") {
        // Convert operands to boolean (0 or 1)
        left = builder->CreateICmpNE(left, llvm::ConstantInt::get(getIntType(), 0), "tobool");
        right = builder->CreateICmpNE(right, llvm::ConstantInt::get(getIntType(), 0), "tobool");
        // Perform logical AND
        auto result = builder->CreateAnd(left, right, "andtmp");
        // Convert back to int32
        return builder->CreateZExt(result, getIntType(), "tozext");
    }
    else if (binary->op == "atau") {
        // Convert operands to boolean (0 or 1)
        left = builder->CreateICmpNE(left, llvm::ConstantInt::get(getIntType(), 0), "tobool");
        right = builder->CreateICmpNE(right, llvm::ConstantInt::get(getIntType(), 0), "tobool");
        // Perform logical OR
        auto result = builder->CreateOr(left, right, "ortmp");
        // Convert back to int32
        return builder->CreateZExt(result, getIntType(), "tozext");
    }
    
    llvm::report_fatal_error(llvm::Twine("Operator biner tidak dikenal: ") + binary->op);
    return nullptr;
}

llvm::Value* Codegen::generateComparison(const ComparisonExpr* comp) {
    llvm::Value* left = generateExpr(comp->left.get());
    llvm::Value* right = generateExpr(comp->right.get());
    
    if (comp->op == "<=") {
        return builder->CreateIntCast(
            builder->CreateICmpSLE(left, right, "cmptmp"),
            getIntType(),
            false
        );
    }
    else if (comp->op == ">=") {
        return builder->CreateIntCast(
            builder->CreateICmpSGE(left, right, "cmptmp"),
            getIntType(),
            false
        );
    }
    else if (comp->op == "<") {
        return builder->CreateIntCast(
            builder->CreateICmpSLT(left, right, "cmptmp"),
            getIntType(),
            false
        );
    }
    else if (comp->op == ">") {
        return builder->CreateIntCast(
            builder->CreateICmpSGT(left, right, "cmptmp"),
            getIntType(),
            false
        );
    }
    else if (comp->op == "adalah") {
        return builder->CreateIntCast(
            builder->CreateICmpEQ(left, right, "eqtmp"),
            getIntType(),
            false
        );
    }
    
    llvm::report_fatal_error(llvm::Twine("Operator perbandingan tidak dikenal: ") + comp->op);
    return nullptr;
}

llvm::Value* Codegen::generateUnary(const UnaryExpr* unary) {
    llvm::Value* operand = generateExpr(unary->operand.get());
    
    if (unary->op == "bukan") {
        return builder->CreateNot(operand, "nottmp");
    }
    
    llvm::report_fatal_error(llvm::Twine("Operator unary tidak dikenal: ") + unary->op);
    return nullptr;
}
}