#include "Parser.hpp"

#include "Lexer.hpp"

LiteralExpr::LiteralExpr(const std::string& t_literalType, const std::string& t_literalValue)
    : literalType{t_literalType}, literalValue{t_literalValue} {}

IdentExpr::IdentExpr(const std::string& t_identName): identName{t_identName} {}

CallExpr::CallExpr(const std::string& t_functionName, std::vector<std::unique_ptr<Expr>> t_args)
    : functionName{t_functionName}, args{std::move(t_args)} {}

BinaryExpr::BinaryExpr(std::unique_ptr<Expr> t_lhs, std::unique_ptr<Expr> t_rhs, const std::string& t_operand):
leftExpr{std::move(t_lhs)}, rightExpr{std::move(t_rhs)}, operand{t_operand} {}



ComparisonExpr::ComparisonExpr(std::unique_ptr<Expr> t_left, std::unique_ptr<Expr> t_right, const std::string& t_operand)
    : leftExpr{std::move(t_left)}, rightExpr{std::move(t_right)}, operand{t_operand} {}

LogicalExpr::LogicalExpr(std::unique_ptr<BoolExpr> t_left, std::unique_ptr<BoolExpr> t_right, const std::string& t_operand)
    : leftBoolExpr{std::move(t_left)}, rightBoolExpr{std::move(t_right)}, operand{t_operand} {}

NotExpr::NotExpr(std::unique_ptr<BoolExpr> t_boolExpr): boolExpr{std::move(t_boolExpr)} {}



VarDeclStmt::VarDeclStmt(bool t_isCond, const std::string& t_typeName, const std::string& t_identName)
    :isConst{t_isCond}, typeName{t_typeName}, identName{t_identName} {}

VarInitStmt::VarInitStmt(std::unique_ptr<Stmt> t_varDecl, std::unique_ptr<Expr> t_initializer)
    : varDecl{std::move(t_varDecl)}, initializer{std::move(t_initializer)} {}

VarAssignStmt::VarAssignStmt(const std::string& t_name, std::unique_ptr<Expr> t_assignment)
    : name{t_name}, assignment{std::move(t_assignment)} {}

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> t_returnExpr): returnExpr{std::move(t_returnExpr)} {}

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> t_stmts): statements{std::move(t_stmts)} {}

FnDeclStmt::FnDeclStmt(const std::string& t_fnName, std::vector<std::unique_ptr<VarDeclStmt>> t_parameters, const std::string& t_returnType, std::unique_ptr<BlockStmt> t_body)
    : functionName{t_fnName}, parameters{std::move(t_parameters)}, returnType{t_returnType}, body{std::move(t_body)} {}

IfStmt::IfStmt(std::unique_ptr<VarInitStmt> t_init, std::unique_ptr<BoolExpr> t_cond, std::unique_ptr<BlockStmt> t_body)
    : initializer{std::move(t_init)}, condition{std::move(t_cond)}, body{std::move(t_body)} {}

CondStmt::CondStmt(std::unique_ptr<IfStmt> ifs, std::vector<std::unique_ptr<IfStmt>> elses, std::unique_ptr<BlockStmt> blk)
    : ifCond{std::move(ifs)}, elsesCond{std::move(elses)}, elseBody{std::move(blk)} {}

ForStmt::ForStmt(std::unique_ptr<Stmt> t_init, std::unique_ptr<BoolExpr> t_cond, std::unique_ptr<VarAssignStmt> t_inc)
    : initializer{std::move(t_init)}, condition{std::move(t_cond)}, assign{std::move(t_inc)} {}

ForRangeStmt::ForRangeStmt(std::unique_ptr<VarDeclStmt> t_init, std::unique_ptr<Expr> t_startExpr, std::unique_ptr<Expr> t_endExpr)
    : varDecl{std::move(t_init)}, startExpression{std::move(t_startExpr)}, endExpression{std::move(t_endExpr)} {}

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
            std::unique_ptr<Stmt> fnCallStmt = std::make_unique<FnDeclStmt>(parseExpression());
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

            auto fnCall = std::make_unique<CallExpr>(functionName, std::move(expressions));

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