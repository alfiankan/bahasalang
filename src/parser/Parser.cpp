#include "Parser.hpp"
#include <stdexcept>

namespace bahasa {

Parser::Parser(const std::vector<std::string>& tokens) : tokens(tokens) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        statements.push_back(parseStatement());
    }
    return statements;
}

StmtPtr Parser::parseStatement() {
    if (match("ABAIKAN")) {
        return parseTryBlock();
    }
    if (match("FUNCTION")) {
        return parseFunctionDeclaration();
    }
    if (match("MUTASI")) {
        return parseVarDeclaration();
    }
    if (match("RETURN_ARROW")) {
        return parseReturnStatement();
    }
    return parseExpressionStatement();
}

StmtPtr Parser::parseTryBlock() {
    consume("{", "Expect '{' after 'abaikan'");
    std::vector<StmtPtr> statements;
    
    // Handle empty block case
    if (check("}")) {
        consume("}", "Expect '}' after try block");
        return std::make_shared<TryStmt>(statements);
    }
    
    while (!check("}") && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume("}", "Expect '}' after try block");
    return std::make_shared<TryStmt>(statements);
}

StmtPtr Parser::parseFunctionDeclaration() {
    std::string name = consume("IDENTIFIER", "Expect function name");
    consume("(", "Expect '(' after function name");
    
    std::vector<Parameter> parameters;
    if (!check(")")) {
        do {
            std::string paramName = consume("IDENTIFIER", "Expect parameter name");
            consume(":", "Expect ':' after parameter name");
            std::string paramType = consume("IDENTIFIER", "Expect parameter type");
            parameters.emplace_back(paramName, paramType);
        } while (match(","));
    }
    
    consume(")", "Expect ')' after parameters");
    consume("->", "Expect '->' after function parameters");
    std::string returnType = consume("IDENTIFIER", "Expect return type");
    consume("{", "Expect '{' before function body");
    
    std::vector<StmtPtr> body;
    while (!check("}") && !isAtEnd()) {
        body.push_back(parseStatement());
    }
    
    consume("}", "Expect '}' after function body");
    return std::make_shared<FunctionStmt>(name, parameters, returnType, body);
}

StmtPtr Parser::parseVarDeclaration() {
    std::string name = consume("IDENTIFIER", "Expect variable name");
    consume(":", "Expect ':' after variable name");
    
    std::shared_ptr<Type> type;
    if (match("koleksi")) {
        consume("[", "Expect '[' after 'koleksi'");
        type = Type::createArray(Type::createInt(), 0); // For now, assuming int array
        consume("]", "Expect ']' after array type");
    } else {
        type = Type::createInt();
    }
    
    consume("=", "Expect '=' after variable declaration");
    ExprPtr initializer = parseExpression();
    
    return std::make_shared<VarDeclStmt>(name, type, initializer);
}

StmtPtr Parser::parseReturnStatement() {
    ExprPtr value = parseExpression();
    return std::make_shared<ReturnStmt>(value);
}

StmtPtr Parser::parseExpressionStatement() {
    ExprPtr expr = parseExpression();
    return std::make_shared<ExprStmt>(expr);
}

ExprPtr Parser::parseExpression() {
    return parseAssignment();
}

ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseEquality();
    
    if (match("=")) {
        std::string name = std::dynamic_pointer_cast<VariableExpr>(expr)->name;
        ExprPtr value = parseAssignment();
        return std::make_shared<AssignmentExpr>(name, value);
    }
    
    return expr;
}

ExprPtr Parser::parseEquality() {
    ExprPtr expr = parseComparison();
    
    while (match("adalah")) {
        std::string op = previous();
        ExprPtr right = parseComparison();
        expr = std::make_shared<ComparisonExpr>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::parseComparison() {
    ExprPtr expr = parseTerm();
    
    while (match("<=") || match(">=") || match("<") || match(">")) {
        std::string op = previous();
        ExprPtr right = parseTerm();
        expr = std::make_shared<ComparisonExpr>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::parseTerm() {
    ExprPtr expr = parseFactor();
    
    while (match("+") || match("-")) {
        std::string op = previous();
        ExprPtr right = parseFactor();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::parseFactor() {
    ExprPtr expr = parseUnary();
    
    while (match("*") || match("/") || match("modulo")) {
        std::string op = previous();
        ExprPtr right = parseUnary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::parseUnary() {
    if (match("-")) {
        std::string op = previous();
        ExprPtr right = parseUnary();
        return std::make_shared<UnaryExpr>(op, right);
    }
    
    return parsePrimary();
}

ExprPtr Parser::parsePrimary() {
    if (match("NUMBER")) {
        return std::make_shared<NumberExpr>(std::stoi(previous()));
    }
    
    if (match("STRING")) {
        return std::make_shared<StringExpr>(previous());
    }
    
    if (match("IDENTIFIER")) {
        std::string name = previous();
        
        if (match("(")) {
            std::vector<ExprPtr> arguments;
            if (!check(")")) {
                do {
                    arguments.push_back(parseExpression());
                } while (match(","));
            }
            consume(")", "Expect ')' after arguments");
            return std::make_shared<CallExpr>(name, arguments);
        }
        
        if (match(".")) {
            ExprPtr index = parseExpression();
            return std::make_shared<ArrayIndexExpr>(name, index);
        }
        
        return std::make_shared<VariableExpr>(name);
    }
    
    if (match("[")) {
        std::vector<ExprPtr> elements;
        if (!check("]")) {
            do {
                elements.push_back(parseExpression());
            } while (match(","));
        }
        consume("]", "Expect ']' after array elements");
        return std::make_shared<ArrayLiteralExpr>(elements);
    }
    
    throw std::runtime_error("Expect expression");
}

bool Parser::match(const std::string& type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(const std::string& type) {
    if (isAtEnd()) return false;
    return peek() == type;
}

std::string Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

std::string Parser::peek() {
    return tokens[current];
}

bool Parser::isAtEnd() {
    return peek() == "EOF";
}

std::string Parser::previous() {
    return tokens[current - 1];
}

std::string Parser::consume(const std::string& type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(message);
}

} // namespace bahasa 