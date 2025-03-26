#ifndef BAHASA_LEXER_HPP
#define BAHASA_LEXER_HPP

#include <string>
#include <vector>

namespace bahasa {

enum class TokenType {
    // Keywords
    FUNCTION,     // function
    INT,          // int type
    MUTASI,       // mutasi (variable declaration)
    IF,           // if
    MODUL,        // modul (module declaration)
    MODULO,       // modulo (modulo operator)
    ADALAH,       // adalah (equality operator)
    TIDUR,        // tidur (sleep function)
    
    // Symbols
    ARROW,        // ->
    LPAREN,       // (
    RPAREN,       // )
    LBRACE,       // {
    RBRACE,       // }
    COMMA,        // ,
    COLON,        // :
    PLUS,         // +
    MINUS,        // -
    MULTIPLY,     // *
    DIVIDE,       // /
    RETURN_ARROW, // <-
    EQUALS,       // =
    LESS_EQUAL,   // <=
    GREATER,      // >
    LESS,         // <
    GREATER_EQUAL, // >=
    DAN,          // dan
    ATAU,         // atau
    
    // Others
    IDENTIFIER,
    NUMBER,
    EOL,          // End of line
    END,          // End of file
    STRING,       // String literal "..."
    
    INVALID
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    
    Token(TokenType t, std::string l, int ln) 
        : type(t), lexeme(std::move(l)), line(ln) {}
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source;
    int current = 0;
    int start = 0;
    int line = 1;
    
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void skipWhitespace();
    Token identifier();
    Token number();
    Token makeToken(TokenType type);
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    Token string();
};

} // namespace bahasa

#endif // BAHASA_LEXER_HPP 