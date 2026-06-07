#include "Parser.hpp"

#include "Lexer.hpp"

BinaryExpr::BinaryExpr(std::unique_ptr<Expr> t_lhs, std::unique_ptr<Expr> t_rhs, const char t_op):
left{std::move(t_lhs)}, right{std::move(t_rhs)}, op{t_op} {}

LiteralExpr::LiteralExpr(): literalValue{"0"} {}

LiteralExpr::LiteralExpr(const std::string& t_literalValue): literalValue{t_literalValue} {}

FnCallExpr::FnCallExpr(const std::string& t_functionName, std::vector<std::unique_ptr<Expr>> t_args)
    : functionName{t_functionName}, args{std::move(t_args)} {}

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> t_returnExpr): returnExpr{std::move(t_returnExpr)} {}

FnCallStmt::FnCallStmt(std::unique_ptr<Expr> t_fnCallExpr): fnCallExpr(std::move(t_fnCallExpr)) {};

VarInitializerStmt::VarInitializerStmt(const std::string& t_typeName, const std::string& t_name, std::unique_ptr<Expr> t_initializer)
    : typeName{t_typeName}, name{t_name}, initializer{std::move(t_initializer)} {}

VarAssignStmt::VarAssignStmt(const std::string& t_name, std::unique_ptr<Expr> t_initializer)
    : name{t_name}, initializer{std::move(t_initializer)} {}

Parser::Parser(const std::vector<Token>& t_tokens): tokens_{t_tokens}, index_{0} {}

bool Parser::checkType(const TokenType t_type) const {
    return tokens_[index_].type == t_type;
}

bool Parser::checkSubType(const TokenSubType t_subType) const {
    return tokens_[index_].subType == t_subType;
}

bool Parser::matchType(const TokenType t_subType) {
    if (checkType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

bool Parser::matchSubType(const TokenSubType t_subType) {
    if (checkSubType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

const Token &Parser::consumeType(const TokenType t_type) {
    if (checkType(t_type)) {
        return tokens_[index_++];
    }

    throw std::runtime_error("Error with consume function");
}

const Token& Parser::consumeSubType(const TokenSubType t_subType) {
    if (checkSubType(t_subType)) {
        return tokens_[index_++];
    }

    throw std::runtime_error("Error with consume function");
}

Program Parser::parse() {
    Program program;

    while (index_ != tokens_.size()) {
        program.functions.push_back(parseFunction());
    }

    return program;
}

Function Parser::parseFunction() {
    consumeSubType(TokenSubType::Func);
    Function fn;

    const auto fnNameToken = consumeType(TokenType::Identifier);
    fn.functionName = fnNameToken.lexeme;

    consumeSubType(TokenSubType::LParen);

    while (not checkSubType(TokenSubType::RParen)) {
        const auto typeToken = consumeType(TokenType::Type);
        const auto paramNameToken = consumeType(TokenType::Identifier);

        fn.parameters.emplace_back(typeToken.lexeme, paramNameToken.lexeme);

        matchSubType(TokenSubType::Comma);
    }

    consumeSubType(TokenSubType::RParen);
    const auto returnTypeToken = consumeType(TokenType::Type);
    fn.returnType = returnTypeToken.lexeme;

    fn.body = parseBlock();

    return fn;
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    consumeSubType(TokenSubType::LBrace);

    auto block = std::make_unique<BlockStmt>();

    while (not matchSubType(TokenSubType::RBrace)) {
        block->statements.push_back(parseStatement());
    }

    return block;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (matchSubType(TokenSubType::Return)) {
        return std::make_unique<ReturnStmt>(parseExpression());
    }

    if (checkType(TokenType::Type)) {
        const auto typeToken= consumeType(TokenType::Type);
        const auto varNameToken = consumeType(TokenType::Identifier);

        const std::string typeName = typeToken.lexeme;
        const std::string name = varNameToken.lexeme;

        std::unique_ptr<Expr> initializer = std::make_unique<LiteralExpr>();

        if (matchSubType(TokenSubType::Assign)) {
            initializer = parseExpression();
        }

        return std::make_unique<VarInitializerStmt>(typeName, name, std::move(initializer));
    }

    if (checkType(TokenType::Identifier)) {
        const auto nameToken = consumeSubType(TokenSubType::Identifier);

        if (matchSubType(TokenSubType::LParen)) {
            index_-=2;
            std::unique_ptr<Stmt> fnCallStmt = std::make_unique<FnCallStmt>(parseExpression());
            return fnCallStmt;
        }

        std::unique_ptr<Expr> initializer = std::make_unique<LiteralExpr>();

        if (matchSubType(TokenSubType::Assign)) {
            initializer = parseExpression();
        }

        return std::make_unique<VarAssignStmt>(nameToken.lexeme, std::move(initializer));
    }

    throw std::runtime_error("Unsupported statement parsing");
}

std::unique_ptr<Expr> Parser::parseExpression() {
    if (checkType(TokenType::Literal)) {
        const auto literalToken = consumeType(TokenType::Literal);
        std::unique_ptr<Expr> leftExpr = std::make_unique<LiteralExpr>(literalToken.lexeme);

        if (not checkType(TokenType::Arithmetic)) {
            return leftExpr;
        }

        const auto operandToken = consumeType(TokenType::Arithmetic);
        const char op = operandToken.lexeme[0];

        std::unique_ptr<Expr> rightExpr = parseExpression();

        return std::make_unique<BinaryExpr>(std::move(leftExpr), std::move(rightExpr), op);
    }

    if (checkType(TokenType::Identifier)) {
        const auto identifierToken = consumeType(TokenType::Identifier);

        if (matchSubType(TokenSubType::LParen)) {
            const auto functionName = identifierToken.lexeme;

            std::vector<std::unique_ptr<Expr>> expressions;

            while (not matchSubType(TokenSubType::RParen)) {
                expressions.push_back(parseExpression());

                matchSubType(TokenSubType::Comma);
            }

            auto fnCall = std::make_unique<FnCallExpr>(functionName, std::move(expressions));

            if (checkType(TokenType::Arithmetic)) {
                const auto operandToken = consumeType(TokenType::Arithmetic);
                const char op = operandToken.lexeme[0];

                std::unique_ptr<Expr> rightExpr = parseExpression();
                return std::make_unique<BinaryExpr>(std::move(fnCall), std::move(rightExpr), op);
            }

            return fnCall;
        }

        if (checkType(TokenType::Arithmetic)) {
            std::unique_ptr<Expr> leftExpr = std::make_unique<LiteralExpr>(identifierToken.lexeme);

            const auto literalToken = consumeType(TokenType::Arithmetic);
            const char op = literalToken.lexeme[0];

            std::unique_ptr<Expr> rightExpr = parseExpression();

            return std::make_unique<BinaryExpr>(std::move(leftExpr), std::move(rightExpr), op);
        }

        return std::make_unique<LiteralExpr>(identifierToken.lexeme);
    }

    throw std::runtime_error("Bad expression type");
}