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
    // Try to parse an assignment first
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().lexeme;
        
        if (match(TokenType::EQUALS)) {
            ExprPtr value = parseExpression();
            return std::make_shared<AssignmentExpr>(name, value);
        }
        
        // If it's not an assignment, rewind and parse as comparison
        current--;  // Rewind the identifier token
        return parseComparison();
    }
    
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
    
    // Handle type declaration
    consume(TokenType::COLON, "Harap ':' setelah nama variabel.");
    std::shared_ptr<Type> type = parseType();
    
    consume(TokenType::EQUALS, "Harap '=' setelah deklarasi tipe.");
    ExprPtr initializer = parseExpression();
    
    return std::make_shared<VarDeclStmt>(name, type, initializer);
}

std::shared_ptr<Type> Parser::parseType() {
    if (match(TokenType::KOLEKSI)) {
        consume(TokenType::LBRACKET, "Harap '[' setelah 'koleksi'.");
        consume(TokenType::INT, "Harap tipe elemen array.");
        consume(TokenType::RBRACKET, "Harap ']' setelah tipe elemen.");
        return Type::createArray(Type::createInt(), 0); // Size will be set later
    }
    consume(TokenType::INT, "Harap tipe variabel.");
    return Type::createInt();
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
        if (match(TokenType::DOT)) {
            return parseArrayIndex(name);
        }
        return std::make_shared<VariableExpr>(name);
    }
    
    if (match(TokenType::LBRACKET)) {
        return parseArrayLiteral();
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

ExprPtr Parser::parseArrayIndex(const std::string& name) {
    if (match(TokenType::NUMBER)) {
        ExprPtr index = std::make_shared<NumberExpr>(std::stoi(previous().lexeme));
        return std::make_shared<ArrayIndexExpr>(name, index);
    }
    error("Harap masukkan indeks array berupa angka");
    return nullptr;
}

ExprPtr Parser::parseArrayLiteral() {
    std::vector<ExprPtr> elements;
    
    if (!check(TokenType::RBRACKET)) {
        do {
            elements.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RBRACKET, "Harap ']' setelah elemen array.");
    return std::make_shared<ArrayLiteralExpr>(elements);
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

void Parser::error(const std::string& message) {
    std::string location = "";
    if (!tokens.empty() && current < tokens.size()) {
        location = " pada baris " + std::to_string(tokens[current].line);
    }
    throw std::runtime_error("Galat" + location + ": " + message);
}

} // namespace bahasa 