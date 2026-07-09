#include "Parser.hpp"

#include "Lexer.hpp"

Parser::Parser(const std::vector<Token>& t_tokens): c{t_tokens} {}

bool Consumer::checkType(const TokenType t_type) const {
    return tokens_[index_].type == t_type;
}

bool Consumer::checkSubType(const TokenSubType t_subType) const {
    return tokens_[index_].subType == t_subType;
}

bool Consumer::matchType(const TokenType t_subType) {
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

const Token& Consumer::consumeType(const TokenType t_type) {
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
bool Consumer::isEnd() const {
    return index_ == tokens_.size();
}

Program Parser::parse() {
    Program program;

    while (not c.isEnd()) {
        program.functions.push_back(parseFunction());
    }

    return program;
}

FnDeclStmt Parser::parseFunction() {
    c.consumeSubType(TokenSubType::Func);

    const auto fnName = c.consumeSubType(TokenSubType::Identifier).lexeme;

    c.consumeSubType(TokenSubType::LParen);

    std::vector<std::unique_ptr<Stmt>> parameters;

    while (not c.checkSubType(TokenSubType::RParen)) {
        std::unique_ptr<Stmt> varDecl = parseVarDecl();
        parameters.emplace_back(std::move(varDecl));

        c.matchSubType(TokenSubType::Comma);
    }

    c.consumeSubType(TokenSubType::RParen);
    const auto returnType= c.consumeType(TokenType::Type).lexeme;

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
    if (c.checkType(TokenType::Type) or c.checkSubType(TokenSubType::Const)) {
        return parseVar();
    }
    if (c.matchSubType(TokenSubType::Return)) {
        return std::make_unique<ReturnStmt>(parseExpr());
    }

    if (c.checkSubType(TokenSubType::If)) {
        return parseCondition();
    }

    if (c.checkSubType(TokenSubType::For)) {
        return parseForStmt();
    }

    throw std::runtime_error("Unsupported statement parsing");
}

std::unique_ptr<Expr> Parser::parseExpr() {
    if (c.checkType(TokenType::Literal)) {
        const auto literalToken = c.consumeType(TokenType::Literal);
        return std::make_unique<LiteralExpr>(literalToken.lexeme, literalToken.lexeme);
    }

    throw std::runtime_error("Bad expression type");
}

std::unique_ptr<BoolExpr> Parser::parseBoolExpression() {
    if (c.matchSubType(TokenSubType::True)) {
        return std::make_unique<BoolLiteral>(true);
    }

    if (c.matchSubType(TokenSubType::False)) {
        return std::make_unique<BoolLiteral>(false);
    }

    throw std::runtime_error("Bad expression type");
}

std::unique_ptr<Stmt> Parser::parseVarTypeStmt() {
    bool isConst{false};

    if (c.matchSubType(TokenSubType::Const)) {
        isConst = true;
    }

    const auto typeName = c.consumeType(TokenType::Type).lexeme;

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
    const auto identifierName = c.consumeSubType(TokenSubType::Identifier).lexeme;

    return std::make_unique<VarDeclStmt>(std::move(varTypeDecl), identifierName);
}

std::unique_ptr<Stmt> Parser::parseVar() {
    std::unique_ptr<Stmt> varDecl = parseVarDecl();

    if (c.matchType(TokenType::AssignOp)) {
        std::unique_ptr<Expr> expr = parseExpr();
        return std::make_unique<VarInitStmt>(std::move(varDecl), std::move(expr));
    }

    return varDecl;
}

std::unique_ptr<Stmt> Parser::parseVarAssign() {
    const auto identifierToken = c.consumeSubType(TokenSubType::Identifier);
    const auto assignToken = c.consumeType(TokenType::AssignOp);

    std::unique_ptr<Expr> expr = parseExpr();
    return std::make_unique<VarAssignStmt>(identifierToken.lexeme, assignToken.lexeme, std::move(expr));
}

std::unique_ptr<ForStmt> Parser::parseForStmt() {
    c.consumeSubType(TokenSubType::For);
    c.consumeSubType(TokenSubType::LParen);

    std::unique_ptr<Stmt> varDecl;

    if (not c.checkSubType(TokenSubType::Semicolon)) {
        varDecl = parseVar();
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

std::unique_ptr<IfStmt> Parser::parseIfStmt() {
    if (c.checkSubType(TokenSubType::If)) {
        c.consumeSubType(TokenSubType::If);
    } else if (c.checkSubType(TokenSubType::Elif)) {
        c.consumeSubType(TokenSubType::Elif);
    }

    c.consumeSubType(TokenSubType::LParen);

    std::unique_ptr<Stmt> initializer;

    if (c.checkType(TokenType::Type) || c.checkSubType(TokenSubType::Const)) {
        initializer = parseVar();
        c.consumeSubType(TokenSubType::Semicolon);
    }

    std::unique_ptr<BoolExpr> condition = parseBoolExpression();
    c.consumeSubType(TokenSubType::RParen);
    std::unique_ptr<BlockStmt> block = parseBlock();

    return std::make_unique<IfStmt>(std::move(initializer), std::move(condition), std::move(block));
}

std::unique_ptr<CondStmt> Parser::parseCondition() {
    auto condition = std::make_unique<CondStmt>();

    auto ifStmt = parseIfStmt();
    condition->addIfStmt(std::move(ifStmt));

    while (c.checkSubType(TokenSubType::Elif)) {
        auto elifStmt = parseIfStmt();
        condition->addElifStmt(std::move(elifStmt));
    }

    c.consumeSubType(TokenSubType::Else);
    std::unique_ptr<BlockStmt> block = parseBlock();
    condition->addElseBlock(std::move(block));

    return condition;
}
