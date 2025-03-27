#include "parser/Parser.hpp"
#include <stdexcept>

namespace bahasa {

std::vector<StmtPtr> Parser::parse() {
    // Parse module declaration first
    if (match(TokenType::MODUL)) {
        parseModuleDecl();
    }

    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        if (match(TokenType::FUNCTION)) {
            statements.push_back(parseFunction());
        } else if (match(TokenType::MUTASI)) {
            statements.push_back(parseVarDecl());
        } else {
            advance(); // Skip unknown tokens for now
        }
    }
    return statements;
}

void Parser::parseModuleDecl() {
    consume(TokenType::IDENTIFIER, "Harap masukkan nama modul.");
    moduleName = previous().lexeme;
}

StmtPtr Parser::parseFunction() {
    // Parse function name
    consume(TokenType::IDENTIFIER, "Harap masukkan nama fungsi.");
    std::string name = previous().lexeme;
    
    // Parse parameters
    consume(TokenType::LPAREN, "Harap '(' setelah nama fungsi.");
    auto params = parseParameters();
    consume(TokenType::RPAREN, "Harap ')' setelah parameter.");
    
    // Parse return type
    consume(TokenType::ARROW, "Harap '->' setelah parameter.");
    consume(TokenType::INT, "Harap tipe kembali.");
    std::string returnType = previous().lexeme;
    
    // Parse body
    consume(TokenType::LBRACE, "Harap '{' sebelum tubuh fungsi.");
    
    std::vector<StmtPtr> body;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match(TokenType::RETURN_ARROW)) {
            auto expr = parseExpression();
            body.push_back(std::make_shared<ReturnStmt>(expr));
        } else if (match(TokenType::MUTASI)) {
            body.push_back(parseVarDecl());
        } else if (match(TokenType::IF)) {
            body.push_back(parseIf());
        } else if (match(TokenType::ABAIKAN)) {
            body.push_back(parseTryBlock());
        } else {
            // Parse expression statement (e.g., function calls)
            auto expr = parseExpression();
            body.push_back(std::make_shared<ExprStmt>(expr));
        }
    }
    
    consume(TokenType::RBRACE, "Harap '}' setelah tubuh fungsi.");
    
    return std::make_shared<FunctionStmt>(name, params, returnType, body);
}

StmtPtr Parser::parseIf() {
    ExprPtr condition = parseExpression();
    
    consume(TokenType::LBRACE, "Harap '{' setelah kondisi if.");
    
    std::vector<StmtPtr> thenBranch;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match(TokenType::RETURN_ARROW)) {
            auto expr = parseExpression();
            thenBranch.push_back(std::make_shared<ReturnStmt>(expr));
        } else if (match(TokenType::MUTASI)) {
            thenBranch.push_back(parseVarDecl());
        } else {
            // Handle function calls and other expressions
            auto expr = parseExpression();
            thenBranch.push_back(std::make_shared<ExprStmt>(expr));
        }
    }
    
    consume(TokenType::RBRACE, "Harap '}' setelah tubuh if.");
    
    return std::make_shared<IfStmt>(condition, thenBranch);
}

StmtPtr Parser::parseTryBlock() {
    consume(TokenType::LBRACE, "Harap '{' setelah 'abaikan'");
    std::vector<StmtPtr> statements;
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match(TokenType::RETURN_ARROW)) {
            auto expr = parseExpression();
            statements.push_back(std::make_shared<ReturnStmt>(expr));
        } else if (match(TokenType::MUTASI)) {
            statements.push_back(parseVarDecl());
        } else if (match(TokenType::IF)) {
            statements.push_back(parseIf());
        } else {
            auto expr = parseExpression();
            statements.push_back(std::make_shared<ExprStmt>(expr));
        }
    }
    
    consume(TokenType::RBRACE, "Harap '}' setelah blok abaikan");
    return std::make_shared<TryStmt>(statements);
}

} // namespace bahasa 