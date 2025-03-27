#include "parser/Parser.hpp"
#include <stdexcept>

namespace bahasa {

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

} // namespace bahasa 