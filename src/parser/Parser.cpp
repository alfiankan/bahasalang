#include "Parser.hpp"
#include <stdexcept>
#include "Parser.hpp"
#include "statements/StatementParser.cpp"
#include "expressions/ExpressionParser.cpp"
#include "types/TypeParser.cpp"
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

void Parser::error(const std::string& message) {
    std::string location = "";
    if (!tokens.empty() && current < tokens.size()) {
        location = " pada baris " + std::to_string(tokens[current].line);
    }
    throw std::runtime_error("Galat" + location + ": " + message);
}

} // namespace bahasa 