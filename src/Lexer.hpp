#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    Identifier,
    Type,
    Keyword,
    Literal,
    Arithmetic,
    Separators,
};

enum class TokenSubType {
    Identifier,

    IntLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,

    Int,
    Uint,
    Float,
    Char,
    Bool,
    Void,

    Func,
    Return,

    Assign,
    Plus,
    Minus,
    Multiply,
    Divide,

    LBrace,
    RBrace,
    LParen,
    RParen,
    Comma,
};

struct Token {
    Token() = delete;
    Token(TokenType, TokenSubType, std::string t_lexeme, int t_line);
    ~Token() noexcept = default;

    TokenType type;
    TokenSubType subType;
    std::string lexeme;
    int line;
};

enum class ParserState{ Regular, StringLiteral /*, MultiLineComment*/};

class TokenParser {
public:
    TokenParser();
    ~TokenParser() noexcept = default;

    [[nodiscard]] std::vector<std::tuple<int, int, std::string>> consumeLine(const std::string&);
private:
    [[nodiscard]] bool isAlphabetic(char) const;
    [[nodiscard]] bool isDigit(char) const;
    [[nodiscard]] bool isNumeric(char) const;
    [[nodiscard]] bool isAlphaNumeric(char) const;
    [[nodiscard]] bool isWord(char) const;
    [[nodiscard]] bool isOperator(char) const;
    [[nodiscard]] bool isBrace(char) const;
    [[nodiscard]] bool isWhiteSpace(char) const;
    [[nodiscard]] bool isString(char) const;
    [[nodiscard]] bool isChar(char) const;
    [[nodiscard]] bool isComment(char, const std::string&, int) const;

    [[nodiscard]] std::tuple<int, std::string> parseWord(const std::string& t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string> parseNumeric(const std::string& t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string> parseOperator(const std::string& t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string> parseBraces(const std::string& t_input, int) const;
    [[nodiscard]] int skipComment(const std::string& t_input, int);
    [[nodiscard]] std::tuple<int, std::string> parseString(const std::string& t_input, int);
    [[nodiscard]] std::tuple<int, std::string> parseChar(const std::string& t_input, int);
    [[nodiscard]] int skipIndentation(const std::string& t_input, int) const;

    friend class TokenIdentifier;

    ParserState parserState;
    std::string stringLiteral;
    int rowIndex;
};

class TokenIdentifier {
public:
    TokenIdentifier();
    ~TokenIdentifier() noexcept = default;

    std::vector<Token> consumeStrToken(const TokenParser&, const std::vector<std::tuple<int, int, std::string>>&) const;
private:
    std::unordered_map<std::string, TokenSubType> keywords;
    std::unordered_map<std::string, TokenSubType> types;
    std::unordered_map<std::string, TokenSubType> arithmetic;
    std::unordered_map<std::string, TokenSubType> separators;

    TokenSubType getNumberType(const TokenParser&, const std::string&) const;
};

class Lexer {
public:
    Lexer() = default;
    ~Lexer() noexcept = default;
    void tokenize(const std::vector<std::string>&);
    std::vector<Token> tokens{};
private:
    TokenIdentifier tokenIdentifier;
    TokenParser tokenParser;
};
