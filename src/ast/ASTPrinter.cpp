#include "ASTPrinter.hpp"

namespace bahasa {

void ASTPrinter::printBranch(const std::string& text, const std::string& prefix, bool isLast) {
    std::cout << prefix;
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << text << std::endl;
}

void ASTPrinter::printStmt(const StmtPtr& stmt, std::string prefix, bool isLast) {
    if (auto func = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
        printBranch("Function: " + func->name, prefix, isLast);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        
        // Print parameters
        for (size_t i = 0; i < func->params.size(); ++i) {
            printBranch("Parameter: " + func->params[i].name + ": " + func->params[i].type,
                       newPrefix, i == func->params.size() - 1);
        }
        
        // Print body
        for (size_t i = 0; i < func->body.size(); ++i) {
            printStmt(func->body[i], newPrefix, i == func->body.size() - 1);
        }
    }
    else if (auto var = std::dynamic_pointer_cast<VarDeclStmt>(stmt)) {
        printBranch("VarDecl: " + var->name, prefix, isLast);
        printExpr(var->initializer, prefix + (isLast ? "    " : "│   "), true);
    }
    else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        printBranch("Return", prefix, isLast);
        printExpr(ret->value, prefix + (isLast ? "    " : "│   "), true);
    }
    else if (auto expr = std::dynamic_pointer_cast<ExprStmt>(stmt)) {
        printExpr(expr->expr, prefix, isLast);
    }
}

void ASTPrinter::printExpr(const ExprPtr& expr, std::string prefix, bool isLast) {
    if (auto num = std::dynamic_pointer_cast<NumberExpr>(expr)) {
        printBranch("Number: " + std::to_string(num->value), prefix, isLast);
    }
    else if (auto var = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        printBranch("Variable: " + var->name, prefix, isLast);
    }
    else if (auto binary = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        printBranch("Binary: " + binary->op, prefix, isLast);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        printExpr(binary->left, newPrefix, false);
        printExpr(binary->right, newPrefix, true);
    }
    else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        printBranch("Call: " + call->callee, prefix, isLast);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            printExpr(call->arguments[i], newPrefix, i == call->arguments.size() - 1);
        }
    }
    else if (auto str = std::dynamic_pointer_cast<StringExpr>(expr)) {
        printBranch("String: \"" + str->value + "\"", prefix, isLast);
    }
    else if (auto comp = std::dynamic_pointer_cast<ComparisonExpr>(expr)) {
        printBranch("Comparison: " + comp->op, prefix, isLast);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        printExpr(comp->left, newPrefix, false);
        printExpr(comp->right, newPrefix, true);
    }
    else if (auto unary = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        printBranch("Unary: " + unary->op, prefix, isLast);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        printExpr(unary->operand, newPrefix, true);
    }
}

} // namespace bahasa 