#ifndef BAHASA_PARSER_HPP
#define BAHASA_PARSER_HPP

#include "Lexer.hpp"
#include "ast/AST.hpp"
#include <vector>
#include <memory>

namespace bahasa {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<StmtPtr> parse();
    std::string getModuleName() const { return moduleName; }

private:
    std::vector<Token> tokens;
    int current = 0;
    std::string moduleName;

    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool consume(TokenType type, const std::string& message);
    void error(const std::string& message);
    
    StmtPtr parseFunction();
    std::vector<Parameter> parseParameters();
    ExprPtr parseExpression();
    ExprPtr parseBinary();
    ExprPtr parsePrimary();
    StmtPtr parseVarDecl();
    ExprPtr parseCall(std::string callee);
    std::vector<ExprPtr> parseArguments();
    StmtPtr parseIf();
    ExprPtr parseComparison();
    void parseModuleDecl();
    std::shared_ptr<Type> parseType();
    ExprPtr parseArrayIndex(const std::string& name);
    ExprPtr parseArrayLiteral();

    // Statement parsing
    StmtPtr parseStatement();
    StmtPtr parseTryBlock();
    StmtPtr parseFunctionDeclaration();
    StmtPtr parseVarDeclaration();
    StmtPtr parseReturnStatement();
    StmtPtr parseExpressionStatement();
};

} // namespace bahasa

#endif // BAHASA_PARSER_HPP 