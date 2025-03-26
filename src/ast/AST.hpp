#ifndef BAHASA_AST_HPP
#define BAHASA_AST_HPP

#include <string>
#include <vector>
#include <memory>

namespace bahasa {

// Forward declarations
class Expr;
class Stmt;
class ASTPrinter;  // Add this forward declaration

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

// Base class for all expressions
class Expr {
public:
    virtual ~Expr() = default;
};

// Number literal expression
class NumberExpr : public Expr {
public:
    int value;
    explicit NumberExpr(int val) : value(val) {}
};

// Variable reference expression
class VariableExpr : public Expr {
public:
    std::string name;
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

// Binary operation expression
class BinaryExpr : public Expr {
public:
    ExprPtr left;
    std::string op;
    ExprPtr right;
    
    BinaryExpr(ExprPtr l, std::string o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

// String literal expression
class StringExpr : public Expr {
public:
    std::string value;
    explicit StringExpr(std::string val) : value(std::move(val)) {}
};

// Base class for all statements
class Stmt {
public:
    virtual ~Stmt() = default;
};

// Function parameter
struct Parameter {
    std::string name;
    std::string type;
    Parameter(std::string n, std::string t) 
        : name(std::move(n)), type(std::move(t)) {}
};

// Function declaration
class FunctionStmt : public Stmt {
public:
    std::string name;
    std::vector<Parameter> params;
    std::string returnType;
    std::vector<StmtPtr> body;
    
    FunctionStmt(std::string n, std::vector<Parameter> p, std::string rt, std::vector<StmtPtr> b)
        : name(std::move(n)), params(std::move(p)), returnType(std::move(rt)), body(std::move(b)) {}
};

// Return statement
class ReturnStmt : public Stmt {
public:
    ExprPtr value;
    explicit ReturnStmt(ExprPtr val) : value(std::move(val)) {}
};

// Add new expression type for function calls
class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<ExprPtr> arguments;
    
    CallExpr(std::string c, std::vector<ExprPtr> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
};

// Add new statement type for variable declarations
class VarDeclStmt : public Stmt {
public:
    std::string name;
    ExprPtr initializer;
    
    VarDeclStmt(std::string n, ExprPtr init)
        : name(std::move(n)), initializer(std::move(init)) {}
};

// Add new statement type for if statements
class IfStmt : public Stmt {
public:
    ExprPtr condition;
    std::vector<StmtPtr> thenBranch;
    
    IfStmt(ExprPtr cond, std::vector<StmtPtr> then)
        : condition(std::move(cond)), thenBranch(std::move(then)) {}
};

// Update ComparisonExpr to handle both comparison and equality
class ComparisonExpr : public Expr {
public:
    ExprPtr left;
    std::string op;  // "<=", "adalah", "modulo"
    ExprPtr right;
    
    ComparisonExpr(ExprPtr l, std::string o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

// Add new statement type for expression statements (like function calls)
class ExprStmt : public Stmt {
public:
    ExprPtr expr;
    explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
};

// Add this class definition
class UnaryExpr : public Expr {
public:
    std::string op;
    ExprPtr operand;
    
    UnaryExpr(std::string o, ExprPtr e)
        : op(std::move(o)), operand(std::move(e)) {}
};

// Move ASTPrinter class definition to ASTPrinter.hpp
// Remove any ASTPrinter declarations from here

} // namespace bahasa

#endif // BAHASA_AST_HPP 