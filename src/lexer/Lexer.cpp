#include "Lexer.hpp"
#include <unordered_map>
#include <stdexcept>

namespace bahasa {

static std::unordered_map<std::string, TokenType> keywords = {
    {"fungsi", TokenType::FUNCTION},
    {"int", TokenType::INT},
    {"mutasi", TokenType::MUTASI},
    {"jika", TokenType::IF},
    {"modul", TokenType::MODUL},
    {"modulo", TokenType::MODULO},
    {"adalah", TokenType::ADALAH},
    {"dan", TokenType::DAN},
    {"atau", TokenType::ATAU},

};

Lexer::Lexer(std::string source) : source(std::move(source)) {}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                start = current; // Reset start position after newline
                break;
            case '-':
                if (peekNext() == '-') {  // Comment starts with --
                    // Advance past both dashes
                    advance();
                    advance();
                    // Skip until end of line
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();
    
    std::string text = source.substr(start, current - start);
    // Trim whitespace from the lexeme
    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);
    TokenType type = TokenType::IDENTIFIER;
    
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        type = it->second;
    }
    
    return Token(type, text, line);
}

Token Lexer::number() {
    while (isDigit(peek())) advance();
    
    std::string num = source.substr(start, current - start);
    // Trim whitespace from the lexeme
    num.erase(0, num.find_first_not_of(" \t\n\r"));
    num.erase(num.find_last_not_of(" \t\n\r") + 1);
    return Token(TokenType::NUMBER, num, line);
}

Token Lexer::string() {
    std::string value;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\\') {
            advance(); // consume backslash
            switch (peek()) {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '"':
                    value += '"';
                    break;
                default:
                    throw std::runtime_error("Urutan escape tidak valid");
            }
            advance();
        } else {
            value += peek();
            advance();
        }
    }
    
    if (isAtEnd()) {
        throw std::runtime_error("String belum ditutup.");
    }
    
    advance(); // Consume the closing "
    return Token(TokenType::STRING, value, line);
}

Token Lexer::makeToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    // Trim whitespace from the lexeme
    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);
    return Token(type, text, line);
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        start = current;
        skipWhitespace();
        
        if (isAtEnd()) break;
        
        char c = advance();
        
        if (isDigit(c)) {
            tokens.push_back(number());
            continue;
        }
        
        if (isAlpha(c)) {
            tokens.push_back(identifier());
            continue;
        }
        
        switch (c) {
            case '(': tokens.push_back(makeToken(TokenType::LPAREN)); break;
            case ')': tokens.push_back(makeToken(TokenType::RPAREN)); break;
            case '{': tokens.push_back(makeToken(TokenType::LBRACE)); break;
            case '}': tokens.push_back(makeToken(TokenType::RBRACE)); break;
            case ',': tokens.push_back(makeToken(TokenType::COMMA)); break;
            case ':': tokens.push_back(makeToken(TokenType::COLON)); break;
            case '+': tokens.push_back(makeToken(TokenType::PLUS)); break;
            case '*': tokens.push_back(makeToken(TokenType::MULTIPLY)); break;
            case '/': tokens.push_back(makeToken(TokenType::DIVIDE)); break;
            case '-':
                if (match('>')) {
                    tokens.push_back(makeToken(TokenType::ARROW));
                } else {
                    tokens.push_back(makeToken(TokenType::MINUS));
                }
                break;
            case '>':
                if (match('=')) {
                    tokens.push_back(makeToken(TokenType::GREATER_EQUAL));
                } else {
                    tokens.push_back(makeToken(TokenType::GREATER));
                }
                break;
            case '<':
                if (match('-')) {
                    tokens.push_back(makeToken(TokenType::RETURN_ARROW));
                } else if (match('=')) {
                    tokens.push_back(makeToken(TokenType::LESS_EQUAL));
                } else {
                    tokens.push_back(makeToken(TokenType::LESS));
                }
                break;
            case '=': tokens.push_back(makeToken(TokenType::EQUALS)); break;
            case '"': tokens.push_back(string()); break;
            default:
                // Handle error
                break;
        }
    }
    
    tokens.push_back(Token(TokenType::END, "", line));
    return tokens;
}

} // namespace bahasa 