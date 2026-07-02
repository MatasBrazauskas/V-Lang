#include "Parser.hpp"

#include "Lexer.hpp"

Parser::Parser(const std::vector<Token>& t_tokens): c{t_tokens} {}

bool Consumer::checkType(const TokenSubType t_type) const {
    return tokens_[index_].type == t_type;
}

bool Consumer::checkSubType(const TokenSubType t_subType) const {
    return tokens_[index_].subType == t_subType;
}

bool Consumer::matchType(const TokenSubType t_subType) {
    if (checkType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

bool Consumer::matchSubType(const TokenSubType t_subType) {
    if (checkSubType(t_subType)) {
        index_++;
        return true;
    }
    return false;
}

bool Consumer::isEnd() const {
    return index_ == tokens_.size();
}

const Token& Consumer::consumeType(const TokenSubType t_type) {
    if (checkType(t_type)) {
        return tokens_[index_++];
    }

    throw std::runtime_error("Error with consume function");
}

const Token& Consumer::consumeSubType(const TokenSubType t_subType) {
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
    c.consumeSubType(TokenSubType::Func);

    const auto fnName = c.consumeType(TokenSubType::Identifier).lexeme;

    c.consumeSubType(TokenSubType::LParen);

    std::vector<std::unique_ptr<Stmt>> parameters;

    while (not c.checkSubType(TokenSubType::RParen)) {
        std::unique_ptr<Stmt> varDecl = parseVarDecl();
        parameters.emplace_back(std::move(varDecl));

        c.matchSubType(TokenSubType::Comma);
    }

    c.consumeSubType(TokenSubType::RParen);
    const auto returnType= c.consumeType(TokenSubType::Type).lexeme;

    std::unique_ptr<Stmt> body = parseBlock();

    return FnDeclStmt{fnName, std::move(parameters), returnType, std::move(body)};
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    c.consumeSubType(TokenSubType::LCurlyBracket);

    std::vector<std::unique_ptr<Stmt>> statements;

    while (not c.matchSubType(TokenSubType::RCurlyBracket)) {
        statements.push_back(parseStatement());
    }

    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (c.matchSubType(TokenSubType::Return)) {
        return std::make_unique<ReturnStmt>(parseExpr());
    }

    if (c.checkSubType(TokenSubType::If)) {

    }

    if (c.checkSubType(TokenSubType::For)) {
        return parseForStmt();
    }

    throw std::runtime_error("Unsupported statement parsing");
}

std::unique_ptr<Expr> Parser::parseExpr() {
    if (c.checkType(TokenSubType::Literal)) {
        const auto literalToken = c.consumeType(TokenSubType::Literal);
        std::unique_ptr<Expr> leftExpr = std::make_unique<LiteralExpr>(literalToken.lexeme, "");
    }

    throw std::runtime_error("Bad expression type");
}

std::unique_ptr<BoolExpr> Parser::parseBoolExpression() {
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parseVarTypeStmt() {
    bool isConst{false};

    if (c.matchSubType(TokenSubType::Const)) {
        isConst = true;
    }

    const auto typeName = c.consumeType(TokenSubType::Type).lexeme;

    auto declType = DeclType::Value;
    std::unique_ptr<Expr> arraySizeExpr = {};

    if (c.matchSubType(TokenSubType::LSquareBracket)) {
        declType = DeclType::Array;
        arraySizeExpr = parseExpr();
        c.consumeSubType(TokenSubType::RSquareBracket);
    } else if (c.matchSubType(TokenSubType::Multiply)) {
        declType = DeclType::Pointer;
    } else if (c.matchSubType(TokenSubType::Reference)) {
        declType = DeclType::Reference;
    }

    return std::make_unique<VarTypeStmt>(isConst, typeName, declType, std::move(arraySizeExpr));
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {
    std::unique_ptr<Stmt> varTypeDecl = parseVarTypeStmt();
    const auto identifierName = c.consumeType(TokenSubType::Identifier).lexeme;

    return std::make_unique<VarDeclStmt>(std::move(varTypeDecl), identifierName);
}

std::unique_ptr<Stmt> Parser::parseVar() {
    std::unique_ptr<Stmt> varDecl = parseVarDecl();

    if (c.matchSubType(TokenSubType::Assign)) {
        std::unique_ptr<Expr> expr = parseExpr();
        return std::make_unique<VarInitStmt>(std::move(varDecl), std::move(expr));
    }

    return varDecl;
}

std::unique_ptr<Stmt> Parser::parseVarAssign() {
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parseForStmt() {
    c.consumeSubType(TokenSubType::For);
    c.consumeSubType(TokenSubType::LParen);

    std::unique_ptr<Stmt> varDecl = parseVarDecl();

    if (c.matchSubType(TokenSubType::Colon)) {
        std::unique_ptr<Expr> startExpr = parseExpr();
        c.consumeSubType(TokenSubType::Range);

        std::unique_ptr<Expr> endExpr = parseExpr();
        c.consumeSubType(TokenSubType::RParen);

        std::unique_ptr<Stmt> block = parseBlock();

        return std::make_unique<ForRangeStmt>(std::move(varDecl), std::move(startExpr), std::move(endExpr), std::move(block));
    }

    if (c.checkSubType(TokenSubType::Assign)) {
        std::unique_ptr<Expr> expr = parseExpr();
        varDecl = std::make_unique<VarInitStmt>(std::move(varDecl), std::move(expr));
    }

    c.consumeSubType(TokenSubType::Semicolon);
    std::unique_ptr<BoolExpr> condition;

    if (not c.checkSubType(TokenSubType::Semicolon)) {
        condition = parseBoolExpression();
    }

    c.consumeSubType(TokenSubType::Semicolon);
    std::unique_ptr<Stmt> incrementStmt;

    if (not c.checkSubType(TokenSubType::RParen)) {
        incrementStmt = parseVarAssign();
    }

    c.consumeSubType(TokenSubType::RParen);

    auto block = parseBlock();
    return std::make_unique<ForStmt>(std::move(varDecl), std::move(condition), std::move(incrementStmt), std::move(block));
}
