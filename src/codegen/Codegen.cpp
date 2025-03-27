#include "Codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

namespace bahasa {

Codegen::Codegen(std::string moduleName) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    
    // Set up WebAssembly target
}


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
        return generateArrayIndex(arrayIndex);
    }
    
    llvm::report_fatal_error(llvm::Twine("Tipe ekspresi tidak dikenal"));
    return nullptr;
}

llvm::Value* Codegen::generateNumber(const NumberExpr* num) {
    return llvm::ConstantInt::get(getIntType(), num->value);
}

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

llvm::Value* Codegen::getStringConstant(const std::string& str) {
    return builder->CreateGlobalStringPtr(str);
}

llvm::Type* Codegen::getIntType() {
    return llvm::Type::getInt32Ty(*context);
}

void Codegen::dump(llvm::raw_ostream& os) const {
    module->print(os, nullptr);
}

llvm::Value* Codegen::generateUnary(const UnaryExpr* unary) {
    llvm::Value* operand = generateExpr(unary->operand.get());
    
    if (unary->op == "bukan") {
        return builder->CreateNot(operand, "nottmp");
    }
    
    llvm::report_fatal_error(llvm::Twine("Operator unary tidak dikenal: ") + unary->op);
    return nullptr;
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

llvm::Value* Codegen::generateArrayIndex(const ArrayIndexExpr* arrayIndex) {
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
            llvm::report_fatal_error("Indeks array di luar batas");
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

} // namespace bahasa 