#ifndef BAHASA_AST_PRINTER_HPP
#define BAHASA_AST_PRINTER_HPP

#include "AST.hpp"
#include <iostream>

namespace bahasa {

class ASTPrinter {
public:
    static void printStmt(const StmtPtr& stmt, std::string prefix = "", bool isLast = true);
    static void printExpr(const ExprPtr& expr, std::string prefix = "", bool isLast = true);
    static void printBranch(const std::string& text, const std::string& prefix = "", bool isLast = true);
};

} // namespace bahasa

#endif // BAHASA_AST_PRINTER_HPP 