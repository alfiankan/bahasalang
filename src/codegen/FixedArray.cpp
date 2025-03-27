#include "codegen/Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {



llvm::Value* Codegen::generateArrayLiteral(const ArrayLiteralExpr* arrayLiteral) {
    // Get the size of the array
    size_t size = arrayLiteral->elements.size();
    
    // Create array type
    llvm::Type* elementType = getIntType(); // For now, only supporting int arrays
    llvm::ArrayType* arrayType = llvm::ArrayType::get(elementType, size);
    
    // Create alloca for the array
    llvm::AllocaInst* arrayAlloca = builder->CreateAlloca(arrayType, nullptr, "array");
    
    // Initialize array elements
    for (size_t i = 0; i < size; i++) {
        // Get element value
        llvm::Value* element = generateExpr(arrayLiteral->elements[i].get());
        
        // Create GEP for the index
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(*context, llvm::APInt(32, 0)),
            llvm::ConstantInt::get(*context, llvm::APInt(32, i))
        };
        llvm::Value* elementPtr = builder->CreateGEP(arrayType, arrayAlloca, indices, "array.element");
        
        // Store the element
        builder->CreateStore(element, elementPtr);
    }
    
    return arrayAlloca;
}

llvm::Value* Codegen::generateArrayIndex(const ArrayIndexExpr* arrayIndex, llvm::BasicBlock* errorBlock) {
    // Get the array pointer
    llvm::Value* arrayPtr = namedValues[arrayIndex->array];
    if (!arrayPtr) {
        llvm::report_fatal_error(llvm::Twine("Array tidak ditemukan: ") + arrayIndex->array);
    }
    
    // Get array type
    llvm::Type* ptrType = arrayPtr->getType();
    if (!ptrType->isPointerTy()) {
        llvm::report_fatal_error(llvm::Twine("Variabel bukan pointer: ") + arrayIndex->array);
    }
    
    // Get array size from the alloca instruction
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(arrayPtr);
    if (!alloca) {
        llvm::report_fatal_error(llvm::Twine("Variabel bukan array: ") + arrayIndex->array);
    }
    
    llvm::ArrayType* arrayType = llvm::dyn_cast<llvm::ArrayType>(alloca->getAllocatedType());
    if (!arrayType) {
        llvm::report_fatal_error(llvm::Twine("Variabel bukan array: ") + arrayIndex->array);
    }
    
    // Get array size
    size_t arraySize = arrayType->getArrayNumElements();
    
    // Get index value - must be a constant number
    if (auto numExpr = dynamic_cast<const NumberExpr*>(arrayIndex->index.get())) {
        if (numExpr->value < 0 || numExpr->value >= arraySize) {
            // Branch to error block if it exists
            if (errorBlock) {
                builder->CreateBr(errorBlock);
            }
            return llvm::ConstantInt::get(getIntType(), 0);
        }
        
        llvm::Value* indexValue = llvm::ConstantInt::get(getIntType(), numExpr->value);
        
        // Create GEP indices
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(*context, llvm::APInt(32, 0)),
            indexValue
        };
        
        // Create GEP instruction
        llvm::Value* elementPtr = builder->CreateGEP(arrayType, arrayPtr, indices, "array.index");
        
        // Load and return the element
        return builder->CreateLoad(arrayType->getArrayElementType(), elementPtr, "array.load");
    }
    
    llvm::report_fatal_error("Indeks array harus berupa angka konstan");
    return nullptr;
}

}