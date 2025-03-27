#include "parser/Parser.hpp"
#include <stdexcept>

namespace bahasa {

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

} // namespace bahasa 