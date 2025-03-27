#ifndef BAHASA_CODEGEN_HPP
#define BAHASA_CODEGEN_HPP

#include "ast/AST.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <memory>

namespace bahasa {

class Codegen {
public:
    Codegen(std::string moduleName);
    void generate(const std::vector<StmtPtr>& statements);
    void dump(llvm::raw_ostream& os) const;
    
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unordered_map<std::string, llvm::Value*> namedValues;
    std::unordered_map<std::string, llvm::Function*> functions;
    
    // Statement generators
    void generateFunction(const FunctionStmt* func);
    void generateReturn(const ReturnStmt* ret, llvm::Function* currentFunction);
    void generateVarDecl(const VarDeclStmt* var, llvm::Function* currentFunction);
    void generateIf(const IfStmt* ifStmt, llvm::Function* currentFunction);
    void generateExprStmt(const ExprStmt* stmt, llvm::Function* currentFunction);
    
    // Expression generators
    llvm::Value* generateExpr(const Expr* expr);
    llvm::Value* generateNumber(const NumberExpr* num);
    llvm::Value* generateVariable(const VariableExpr* var);
    llvm::Value* generateBinary(const BinaryExpr* binary);
    llvm::Value* generateComparison(const ComparisonExpr* comp);
    llvm::Value* generateCall(const CallExpr* call);
    llvm::Value* generateUnary(const UnaryExpr* unary);
    llvm::Value* generateAssignment(const AssignmentExpr* assign);
    llvm::Value* generateArrayLiteral(const ArrayLiteralExpr* arrayLiteral);
    llvm::Value* generateArrayIndex(const ArrayIndexExpr* arrayIndex);
    
    // Helper methods
    llvm::Type* getIntType();
    llvm::Function* getCurrentFunction() const;
    void createPrintFunction();
    void createTidurFunction();
    llvm::Value *getStringConstant(const std::string &str);

    // Add these two method declarations
    bool containsPrintCall(const StmtPtr& stmt);
    bool containsSleepCall(const StmtPtr& stmt);
};

} // namespace bahasa

#endif // BAHASA_CODEGEN_HPP 