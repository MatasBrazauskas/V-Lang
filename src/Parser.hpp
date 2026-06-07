#pragma once

#include <memory>
#include <vector>

enum class TokenType;
enum class TokenSubType;
struct Token;

class Expr {
public:
    virtual ~Expr() noexcept = default;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr() = delete;
    explicit BinaryExpr(std::unique_ptr<Expr> t_lhs, std::unique_ptr<Expr> t_rhs, char t_op);

    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    char op;
};

class LiteralExpr : public Expr {
public:
    LiteralExpr();
    explicit LiteralExpr(const std::string& t_literalValue);
    std::string literalValue;
};

class FnCallExpr: public Expr {
public:
    FnCallExpr() = delete;
    explicit FnCallExpr(const std::string&, std::vector<std::unique_ptr<Expr>>);

    std::string functionName;
    std::vector<std::unique_ptr<Expr>> args;
};

class Stmt {
public:
    virtual ~Stmt() noexcept = default;
};

class VarInitializerStmt : public Stmt {
public:
    VarInitializerStmt() = delete;
    VarInitializerStmt(const std::string& t_typeName, const std::string& t_name, std::unique_ptr<Expr> t_initializer);

    std::string typeName;
    std::string name;
    std::unique_ptr<Expr> initializer;
};

class VarAssignStmt : public Stmt {
public:
    VarAssignStmt() = delete;
    VarAssignStmt(const std::string& t_name, std::unique_ptr<Expr> t_initializer);

    std::string name;
    std::unique_ptr<Expr> initializer;
};

class ReturnStmt : public Stmt {
public:
    ReturnStmt() = delete;
    explicit ReturnStmt(std::unique_ptr<Expr>);
    std::unique_ptr<Expr> returnExpr;
};

class FnCallStmt : public Stmt {
public:
    FnCallStmt() = delete;
    explicit FnCallStmt(std::unique_ptr<Expr>);

    std::unique_ptr<Expr> fnCallExpr;
};

class CondStmt : public Stmt {
public:

};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct Parameter {
    std::string type;
    std::string name;
};

class Function {
public:
    std::string functionName;
    std::string returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<BlockStmt> body;
};

class Program {
public:
    std::vector<Function> functions;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>&);
    ~Parser() noexcept = default;
    Program parse();
private:
    [[nodiscard]] bool checkType(TokenType) const;
    [[nodiscard]] bool checkSubType(TokenSubType) const;

    [[nodiscard]] bool matchType(TokenType);
    [[nodiscard]] bool matchSubType(TokenSubType);

    const Token& consumeType(TokenType);
    const Token& consumeSubType(TokenSubType);

    Function parseFunction();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Expr> parseExpression();

    std::vector<Token> tokens_;
    size_t index_;
};