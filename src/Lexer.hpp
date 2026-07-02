#pragma once

#include <expected>
#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    None,
    Literal,
    Type,
    ArithmeticOp,
    AssignOp,
    ComparisonOp,
    LogicalOp,
};

enum class TokenSubType {
    Identifier,

    IntLiteral,
    UintLiteral,
    LongLiteral,
    ULongLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,

    True,
    False,

    Int,
    Uint,
    Long,
    ULong,
    Float,
    Char,
    Bool,
    Void,

    Const,
    Func,
    Return,
    If,
    Elif,
    Else,
    For,
    Default,

    Assign,
    Plus,
    AssignPlus,
    Minus,
    AssignMinus,
    Multiply,
    AssignMultiply,
    Divide,
    AssignDivide,
    Module,
    AssignModule,

    Less,
    More,
    LessEq,
    MoreEq,
    Equals,
    NotEquals,
    And,
    Or,
    Not,

    LCurlyBracket,
    RCurlyBracket,
    LParen,
    RParen,
    LSquareBracket,
    RSquareBracket,
    Comma,
    Semicolon,
    Colon,
    Range,
    Reference,

    EndOfEnum,
};

class CharIdentifier {
public:
    CharIdentifier() = default;
    ~CharIdentifier() noexcept = default;

    static bool isAlphabetic(char);
    static  bool isDigit(char);
    static  bool isWord(char);
    static  bool isOperator(char);
    static  bool isBrace(char);
    static bool isSeparator(char);
    static bool isWhiteSpace(char);
    static bool isString(char);
    static bool isChar(char);
    static bool isComment(char, std::string_view, int);
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

class TokenIdentifier {
public:
    TokenIdentifier();
    ~TokenIdentifier() noexcept = default;
    [[nodiscard]] std::expected<TokenSubType, std::string> parseNumberLiteral(std::string_view) const;
    [[nodiscard]] TokenSubType parseWord(std::string_view) const;
private:


    std::unordered_map<std::string_view, TokenSubType> keywords;
    std::unordered_map<std::string_view, TokenSubType> types;
    std::unordered_map<std::string_view, TokenSubType> arithmeticOp;
    std::unordered_map<std::string_view, TokenSubType> assignOp;
    std::unordered_map<std::string_view, TokenSubType> comparisonOp;
    std::unordered_map<std::string_view, TokenSubType> logicalOp;
    std::unordered_map<std::string_view, TokenSubType> separators;
};

class TokenParser {
public:
    TokenParser();
    ~TokenParser() noexcept = default;

    [[nodiscard]] std::vector<Token> consumeLine(const TokenIdentifier&, std::string_view);
private:

    [[nodiscard]] std::tuple<int, std::string_view> generateWord(std::string_view t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string_view> generateNumber(std::string_view t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string_view> generateOperator(std::string_view t_input, int) const;
    [[nodiscard]] std::tuple<int, std::string_view> parseSeparator(const std::string& t_input, int) const;
    [[nodiscard]] int skipComment(const std::string& t_input, int);
    [[nodiscard]] std::tuple<int, std::string> parseString(const std::string& t_input, int);
    [[nodiscard]] std::tuple<int, std::string> parseChar(const std::string& t_input, int);
    [[nodiscard]] int skipIndentation(const std::string& t_input, int) const;

    ParserState parserState;
    std::string stringLiteral;
    int rowIndex;
};

class Lexer {
public:
    Lexer();
    ~Lexer() noexcept = default;
    void tokenize(std::string_view);
private:
    std::vector<Token> tokens;
    TokenParser tokenParser;
    TokenIdentifier tokenIdentifier;
};
