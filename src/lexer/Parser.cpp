#include "Parser.hpp"
#include <stdexcept>

namespace bahasa {

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || tokens[current].type == TokenType::END;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return true;
    }
    error(message);
    return false;
}

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
        } else {
            // Parse expression statement (e.g., function calls)
            auto expr = parseExpression();
            body.push_back(std::make_shared<ExprStmt>(expr));
            // Skip any remaining tokens until we hit a known statement start
            while (!check(TokenType::RETURN_ARROW) && 
                   !check(TokenType::MUTASI) && 
                   !check(TokenType::IF) && 
                   !check(TokenType::RBRACE) && 
                   !isAtEnd()) {
                advance();
            }
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

std::vector<Parameter> Parser::parseParameters() {
    std::vector<Parameter> params;
    
    if (!check(TokenType::RPAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Harap masukkan nama parameter.");
            std::string paramName = previous().lexeme;
            
            consume(TokenType::COLON, "Harap ':' setelah nama parameter.");
            consume(TokenType::INT, "Harap tipe parameter.");
            std::string paramType = previous().lexeme;
            
            params.emplace_back(paramName, paramType);
        } while (match(TokenType::COMMA));
    }
    
    return params;
}

ExprPtr Parser::parseExpression() {
    return parseComparison();
}

ExprPtr Parser::parseComparison() {
    ExprPtr expr = parseBinary();
    
    while (match(TokenType::LESS) || match(TokenType::GREATER) ||
           match(TokenType::LESS_EQUAL) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::ADALAH)) {
        std::string op = previous().lexeme;
        ExprPtr right = parseBinary();
        expr = std::make_shared<ComparisonExpr>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::parseBinary() {
    ExprPtr expr = parsePrimary();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS) ||
           match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) ||
           match(TokenType::MODULO) || match(TokenType::DAN) ||
           match(TokenType::ATAU)) {
        std::string op = previous().lexeme;
        ExprPtr right = parsePrimary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

StmtPtr Parser::parseVarDecl() {
    consume(TokenType::IDENTIFIER, "Harap masukkan nama variabel setelah 'mutasi'.");
    std::string name = previous().lexeme;
    
    consume(TokenType::EQUALS, "Harap '=' setelah nama variabel.");
    ExprPtr initializer = parseExpression();
    
    return std::make_shared<VarDeclStmt>(name, initializer);
}

ExprPtr Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) {
        return std::make_shared<NumberExpr>(std::stoi(previous().lexeme));
    }
    
    if (match(TokenType::STRING)) {
        return std::make_shared<StringExpr>(previous().lexeme);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().lexeme;
        if (match(TokenType::LPAREN)) {
            return parseCall(name);
        }
        return std::make_shared<VariableExpr>(name);
    }
    
    if (match(TokenType::LPAREN)) {
        ExprPtr expr = parseExpression();
        if (!consume(TokenType::RPAREN, "Harap ')' setelah ekspresi.")) {
            error("Harap ')' setelah ekspresi");
        }
        return expr;
    }
    
    error("Harap masukkan ekspresi");
    return nullptr;
}

ExprPtr Parser::parseCall(std::string callee) {
    auto arguments = parseArguments();
    consume(TokenType::RPAREN, "Harap ')' setelah argumen.");
    return std::make_shared<CallExpr>(callee, arguments);
}

std::vector<ExprPtr> Parser::parseArguments() {
    std::vector<ExprPtr> arguments;
    
    if (!check(TokenType::RPAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    return arguments;
}

void Parser::error(const std::string& message) {
    std::string location = "";
    if (!tokens.empty() && current < tokens.size()) {
        location = " pada baris " + std::to_string(tokens[current].line);
    }
    throw std::runtime_error("Galat" + location + ": " + message);
}

} // namespace bahasa 