#ifndef BAHASA_PARSER_HPP
#define BAHASA_PARSER_HPP

#include "../ast/AST.hpp"
#include <vector>
#include <string>

namespace bahasa {

class Parser {
public:
    explicit Parser(const std::vector<std::string>& tokens);
    
    // Main parsing function
    std::vector<StmtPtr> parse();
    
private:
    // Statement parsing
    StmtPtr parseStatement();
    StmtPtr parseTryBlock();
    StmtPtr parseFunctionDeclaration();
    StmtPtr parseVarDeclaration();
    StmtPtr parseReturnStatement();
    StmtPtr parseExpressionStatement();
    
    // Expression parsing
    ExprPtr parseExpression();
    ExprPtr parseAssignment();
    ExprPtr parseEquality();
    ExprPtr parseComparison();
    ExprPtr parseTerm();
    ExprPtr parseFactor();
    ExprPtr parseUnary();
    ExprPtr parsePrimary();
    
    // Helper functions
    bool match(const std::string& type);
    bool check(const std::string& type);
    std::string advance();
    std::string peek();
    bool isAtEnd();
    std::string previous();
    std::string consume(const std::string& type, const std::string& message);
    
    std::vector<std::string> tokens;
    int current = 0;
};

} // namespace bahasa

#endif // BAHASA_PARSER_HPP 