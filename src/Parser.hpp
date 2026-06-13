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

class LiteralExpr : public Expr {
public:
    LiteralExpr() = delete;
    explicit LiteralExpr(const std::string& t_literalType, const std::string& t_literalValue = "0");
    std::string literalType;
    std::string literalValue;
};

class IdentExpr : public Expr {
public:
    IdentExpr() = delete;
    explicit IdentExpr(const std::string& t_identName);
    std::string identName;
};

class CallExpr: public Expr {
public:
    CallExpr() = delete;
    explicit CallExpr(const std::string&, std::vector<std::unique_ptr<Expr>>);

    std::string functionName;
    std::vector<std::unique_ptr<Expr>> args;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr() = delete;
    explicit BinaryExpr(std::unique_ptr<Expr> t_lhs, std::unique_ptr<Expr> t_rhs, const std::string& t_operand);

    std::unique_ptr<Expr> leftExpr;
    std::unique_ptr<Expr> rightExpr;
    std::string operand;
};

class BoolExpr {
public:
    virtual ~BoolExpr() noexcept = default;
};

class ComparisonExpr : public BoolExpr {
public:
    ComparisonExpr() = delete;
    explicit ComparisonExpr(std::unique_ptr<Expr> t_left, std::unique_ptr<Expr> t_right, const std::string& t_operand);

    std::unique_ptr<Expr> leftExpr;
    std::unique_ptr<Expr> rightExpr;
    std::string operand;
};

class LogicalExpr : public BoolExpr {
public:
    LogicalExpr() = delete;
    explicit LogicalExpr(std::unique_ptr<BoolExpr> t_left, std::unique_ptr<BoolExpr> t_right, const std::string& t_operand);

    std::unique_ptr<BoolExpr> leftBoolExpr;
    std::unique_ptr<BoolExpr> rightBoolExpr;
    std::string operand;
};

class NotExpr : public BoolExpr {
public:
    NotExpr() = delete;
    explicit NotExpr(std::unique_ptr<BoolExpr>);
    std::unique_ptr<BoolExpr> boolExpr;
};

class Stmt {
public:
    virtual ~Stmt() noexcept = default;
};

class VarDeclStmt : public Stmt {
public:
    VarDeclStmt() = delete;
    explicit VarDeclStmt(bool t_isCond, const std::string& t_typeName, const std::string& t_identName);

    bool isConst;
    std::string typeName;
    std::unique_ptr<Expr> initializer;
    std::string identName;
};

class VarInitStmt : public Stmt {
public:
    VarInitStmt() = delete;
    explicit VarInitStmt(std::unique_ptr<Stmt> t_varDecl, std::unique_ptr<Expr> t_initializer);

    std::unique_ptr<Stmt> varDecl;
    std::unique_ptr<Expr> initializer;
};

class VarAssignStmt : public Stmt {
public:
    VarAssignStmt() = delete;
    explicit VarAssignStmt(const std::string& t_name, std::unique_ptr<Expr> t_assignment);

    std::string name;
    std::unique_ptr<Expr> assignment;
};

class ReturnStmt : public Stmt {
public:
    ReturnStmt() = delete;
    explicit ReturnStmt(std::unique_ptr<Expr>);
    std::unique_ptr<Expr> returnExpr;
};

class BlockStmt : public Stmt {
public:
    BlockStmt() = delete;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>>);
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct Parameter {
    std::string type;
    std::string name;
};

class FnDeclStmt : public Stmt {
public:
    FnDeclStmt() = delete;
    explicit FnDeclStmt(const std::string& t_fnName, std::vector<std::unique_ptr<VarDeclStmt>> t_parameters, const std::string& t_returnType, std::unique_ptr<BlockStmt> t_body);

    std::string functionName;
    std::vector<std::unique_ptr<VarDeclStmt>> parameters;
    std::string returnType;
    std::unique_ptr<BlockStmt> body;
};

class IfStmt : public Stmt {
public:
    IfStmt() = delete;
    explicit IfStmt(std::unique_ptr<VarInitStmt> t_init, std::unique_ptr<BoolExpr> t_cond, std::unique_ptr<BlockStmt> t_body);

    std::unique_ptr<VarInitStmt> initializer;
    std::unique_ptr<BoolExpr> condition;
    std::unique_ptr<BlockStmt> body;
};

class CondStmt : public Stmt {
public:
    CondStmt() = delete;
    explicit CondStmt(std::unique_ptr<IfStmt>, std::vector<std::unique_ptr<IfStmt>>, std::unique_ptr<BlockStmt>);
    std::unique_ptr<IfStmt> ifCond;
    std::vector<std::unique_ptr<IfStmt>> elsesCond;
    std::unique_ptr<BlockStmt> elseBody;
};

class ForStmt : public Stmt {
public:
    ForStmt() = delete;
    explicit ForStmt(std::unique_ptr<Stmt>, std::unique_ptr<BoolExpr>, std::unique_ptr<VarAssignStmt>);
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<BoolExpr> condition;
    std::unique_ptr<VarAssignStmt> assign;
};

class ForRangeStmt : public Stmt {
public:
    ForRangeStmt() = delete;
    explicit ForRangeStmt(std::unique_ptr<VarDeclStmt>, std::unique_ptr<Expr>, std::unique_ptr<Expr>);
    std::unique_ptr<VarDeclStmt> varDecl;
    std::unique_ptr<Expr> startExpression;
    std::unique_ptr<Expr> endExpression;
};

class Program {
public:
    Program() = default;
    std::vector<FnDeclStmt> functions;
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

    FnDeclStmt parseFunction();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Expr> parseExpression();

    std::vector<Token> tokens_;
    size_t index_;
};