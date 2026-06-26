#include "Parser.hpp"

#include "Lexer.hpp"

Parser::Parser(const std::vector<Token>& t_tokens): c{t_tokens} {}

bool Consumer::checkType(const TokenType t_type) const {
    return tokens_[index_].type == t_type;
}

bool Consumer::checkSubType(const TokenType t_subType) const {
    return tokens_[index_].subType == t_subType;
}

bool Consumer::matchType(const TokenType t_subType) {
    if (checkType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

bool Consumer::matchSubType(const TokenType t_subType) {
    if (checkSubType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

bool Consumer::isEnd() const {
    return index_ == tokens_.size();
}

const Token& Consumer::consumeType(const TokenType t_type) {
    if (checkType(t_type)) {
        return tokens_[index_++];
    }

    throw std::runtime_error("Error with consume function");
}

const Token& Consumer::consumeSubType(const TokenType t_subType) {
    if (checkSubType(t_subType)) {
        return tokens_[index_++];
    }

    throw std::runtime_error("Error with consume function");
}

Program Parser::parse() {
    Program program;

    while (c.isEnd()) {
        program.functions.push_back(parseFunction());
    }

    return program;
}

FnDeclStmt Parser::parseFunction() {
    c.consumeSubType(TokenType::Func);

    const auto fnName = c.consumeType(TokenType::Identifier).lexeme;

    c.consumeSubType(TokenType::LParen);

    std::vector<std::unique_ptr<Stmt>> parameters;

    while (not c.checkSubType(TokenType::RParen)) {
        std::unique_ptr<Stmt> varDecl = parseVarDecl();
        parameters.emplace_back(std::move(varDecl));

        c.matchSubType(TokenType::Comma);
    }

    c.consumeSubType(TokenType::RParen);
    const auto returnType= c.consumeType(TokenType::Type).lexeme;

    std::unique_ptr<Stmt> body = parseBlock();

    return FnDeclStmt{fnName, std::move(parameters), returnType, std::move(body)};
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    c.consumeSubType(TokenType::LCurlyBracket);

    std::vector<std::unique_ptr<Stmt>> statements;

    while (not c.matchSubType(TokenType::RCurlyBracket)) {
        statements.push_back(parseStatement());
    }

    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (c.matchSubType(TokenType::Return)) {
        return std::make_unique<ReturnStmt>(parseExpr());
    }

    if (c.checkSubType(TokenType::If)) {

    }

    if (c.checkSubType(TokenType::For)) {
        return parseForStmt();
    }

    throw std::runtime_error("Unsupported statement parsing");
}

std::unique_ptr<Expr> Parser::parseExpr() {
    if (c.checkType(TokenType::Literal)) {
        const auto literalToken = c.consumeType(TokenType::Literal);
        std::unique_ptr<Expr> leftExpr = std::make_unique<LiteralExpr>(literalToken.lexeme, "");

        if (not c.checkType(TokenType::Arithmetic)) {
            return leftExpr;
        }

        const auto operandToken = c.consumeType(TokenType::Arithmetic);
        const auto op = operandToken.lexeme;

        std::unique_ptr<Expr> rightExpr = parseExpr();

        return std::make_unique<BinaryExpr>(std::move(leftExpr), std::move(rightExpr), op);
    }

    if (c.checkType(TokenType::Identifier)) {
        const auto identifierToken = c.consumeType(TokenType::Identifier);

        if (c.matchSubType(TokenType::LParen)) {
            const auto functionName = identifierToken.lexeme;

            std::vector<std::unique_ptr<Expr>> expressions;

            while (not c.matchSubType(TokenType::RParen)) {
                expressions.push_back(parseExpr());

                c.matchSubType(TokenType::Comma);
            }

            auto fnCall = std::make_unique<CallExpr>(functionName, std::move(expressions));

            if (c.checkType(TokenType::Arithmetic)) {
                const auto operandToken = c.consumeType(TokenType::Arithmetic);
                const auto op = operandToken.lexeme;

                std::unique_ptr<Expr> rightExpr = parseExpr();
                return std::make_unique<BinaryExpr>(std::move(fnCall), std::move(rightExpr), op);
            }

            return fnCall;
        }

        if (c.checkType(TokenType::Arithmetic)) {
            std::unique_ptr<Expr> leftExpr = std::make_unique<LiteralExpr>(identifierToken.lexeme, "");

            const auto literalToken = c.consumeType(TokenType::Arithmetic);
            const auto op = literalToken.lexeme;

            std::unique_ptr<Expr> rightExpr = parseExpr();

            return std::make_unique<BinaryExpr>(std::move(leftExpr), std::move(rightExpr), op);
        }

        return std::make_unique<LiteralExpr>(identifierToken.lexeme, "");
    }

    throw std::runtime_error("Bad expression type");
}

std::unique_ptr<BoolExpr> Parser::parseBoolExpression() {
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parseVarTypeStmt() {
    bool isConst{false};

    if (c.matchSubType(TokenType::Const)) {
        isConst = true;
    }

    const auto typeName = c.consumeType(TokenType::Type).lexeme;

    auto declType = DeclType::Value;
    std::unique_ptr<Expr> arraySizeExpr = {};

    if (c.matchSubType(TokenType::LSquareBracket)) {
        declType = DeclType::Array;
        arraySizeExpr = parseExpr();
        c.consumeSubType(TokenType::RSquareBracket);
    } else if (c.matchSubType(TokenType::Multiply)) {
        declType = DeclType::Pointer;
    } else if (c.matchSubType(TokenType::Reference)) {
        declType = DeclType::Reference;
    }

    return std::make_unique<VarTypeStmt>(isConst, typeName, declType, std::move(arraySizeExpr));
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {
    std::unique_ptr<Stmt> varTypeDecl = parseVarTypeStmt();
    const auto identifierName = c.consumeType(TokenType::Identifier).lexeme;

    return std::make_unique<VarDeclStmt>(std::move(varTypeDecl), identifierName);
}

std::unique_ptr<Stmt> Parser::parseVar() {
    std::unique_ptr<Stmt> varDecl = parseVarDecl();

    if (c.matchSubType(TokenType::Assign)) {
        std::unique_ptr<Expr> expr = parseExpr();
        return std::make_unique<VarInitStmt>(std::move(varDecl), std::move(expr));
    }

    return varDecl;
}

std::unique_ptr<Stmt> Parser::parseVarAssign() {
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parseForStmt() {
    c.consumeSubType(TokenType::For);
    c.consumeSubType(TokenType::LParen);

    std::unique_ptr<Stmt> varDecl = parseVarDecl();

    if (c.matchSubType(TokenType::Colon)) {
        std::unique_ptr<Expr> startExpr = parseExpr();
        c.consumeSubType(TokenType::Range);

        std::unique_ptr<Expr> endExpr = parseExpr();
        c.consumeSubType(TokenType::RParen);

        std::unique_ptr<Stmt> block = parseBlock();

        return std::make_unique<ForRangeStmt>(std::move(varDecl), std::move(startExpr), std::move(endExpr), std::move(block));
    }

    if (c.checkSubType(TokenType::Assign)) {
        std::unique_ptr<Expr> expr = parseExpr();
        varDecl = std::make_unique<VarInitStmt>(std::move(varDecl), std::move(expr));
    }

    c.consumeSubType(TokenType::Semicolon);
    std::unique_ptr<BoolExpr> condition;

    if (not c.checkSubType(TokenType::Semicolon)) {
        condition = parseBoolExpression();
    }

    c.consumeSubType(TokenType::Semicolon);
    std::unique_ptr<Stmt> incrementStmt;

    if (not c.checkSubType(TokenType::RParen)) {
        incrementStmt = parseVarAssign();
    }

    c.consumeSubType(TokenType::RParen);

    auto block = parseBlock();
    return std::make_unique<ForStmt>(std::move(varDecl), std::move(condition), std::move(incrementStmt), std::move(block));
}
