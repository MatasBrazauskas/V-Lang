#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class TokenType {
    Literal,
    Keyword,
    Type,
    ArithmeticOp,
    AssignOp,
    ComparisonOp,
    LogicalOp,
    Separator,
    None,
};

enum class TokenSubType {
    Const,
    Func,
    Return,
    If,
    Elif,
    Else,
    For,
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

    Plus,
    Minus,
    Multiply,
    Divide,
    Module,

    Assign,
    AssignPlus,
    AssignMinus,
    AssignMultiply,
    AssignDivide,
    AssignModule,

    Equals,
    NotEquals,
    Less,
    More,
    LessEq,
    MoreEq,

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
    Default,

    EndOfEnum,

    Identifier,

    IntLiteral,
    UintLiteral,
    LongLiteral,
    ULongLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
};

struct Token {
    Token(TokenType type, TokenSubType subType, std::string_view lexeme, int line);

    TokenType type;
    TokenSubType subType;
    std::string lexeme;
    int line;
};

class Lexer {
public:
    Lexer();
    ~Lexer();

    void tokenize(std::string_view source) const;
    void clear() const;

    [[nodiscard]] const std::vector<Token>& getTokens() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};