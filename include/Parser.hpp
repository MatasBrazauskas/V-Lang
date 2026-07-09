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
    explicit LiteralExpr(const std::string& t_literalType, const std::string& t_literalValue)
        : literalType{t_literalType}, literalValue{t_literalValue} {}
    std::string literalType;
    std::string literalValue;
};

class IdentExpr : public Expr {
public:
    IdentExpr() = delete;
    explicit IdentExpr(const std::string& t_identName):
        identName{t_identName} {}
    std::string identName;
};

class CallExpr: public Expr {
public:
    CallExpr() = delete;
    explicit CallExpr(const std::string& t_funcName, std::vector<std::unique_ptr<Expr>> t_args):
        functionName{t_funcName}, args{std::move(t_args)} {}

    std::string functionName;
    std::vector<std::unique_ptr<Expr>> args;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr() = delete;
    explicit BinaryExpr(std::unique_ptr<Expr> t_lhs, std::unique_ptr<Expr> t_rhs, const std::string& t_operand):
        leftExpr{std::move(t_lhs)}, rightExpr{std::move(t_rhs)}, operand{t_operand} {}

    std::unique_ptr<Expr> leftExpr;
    std::unique_ptr<Expr> rightExpr;
    std::string operand;
};

class BoolExpr {
public:
    virtual ~BoolExpr() noexcept = default;
};

class BoolLiteral : public BoolExpr {
public:
    BoolLiteral() = delete;
    explicit BoolLiteral(const bool t_isTrue): isTrue{t_isTrue} {}
    bool isTrue;
};

class ComparisonExpr : public BoolExpr {
public:
    ComparisonExpr() = delete;
    explicit ComparisonExpr(std::unique_ptr<Expr> t_left, std::unique_ptr<Expr> t_right, const std::string& t_operand):
        leftExpr{std::move(t_left)}, rightExpr{std::move(t_right)}, operand{t_operand} {}

    std::unique_ptr<Expr> leftExpr;
    std::unique_ptr<Expr> rightExpr;
    std::string operand;
};

class LogicalExpr : public BoolExpr {
public:
    LogicalExpr() = delete;
    explicit LogicalExpr(std::unique_ptr<BoolExpr> t_left, std::unique_ptr<BoolExpr> t_right, const std::string& t_operand):
        leftBoolExpr{std::move(t_left)}, rightBoolExpr{std::move(t_right)}, operand{t_operand} {}

    std::unique_ptr<BoolExpr> leftBoolExpr;
    std::unique_ptr<BoolExpr> rightBoolExpr;
    std::string operand;
};

class NotExpr : public BoolExpr {
public:
    NotExpr() = delete;
    explicit NotExpr(std::unique_ptr<BoolExpr> t_boolExpr): boolExpr{std::move(t_boolExpr)} {}
    std::unique_ptr<BoolExpr> boolExpr;
};

class Stmt {
public:
    virtual ~Stmt() noexcept = default;
};

enum class DeclType {
    Value,
    Pointer,
    Reference,
    Array
};

class VarTypeStmt : public Stmt {
public:
    VarTypeStmt() = delete;
    explicit VarTypeStmt(const bool t_isCond, const std::string &t_typeName, const DeclType t_declType, std::unique_ptr<Expr> t_arrSize)
        :isConst{t_isCond}, typeName{t_typeName}, declarationType{t_declType}, arraySize{std::move(t_arrSize)} {}

    bool isConst;
    std::string typeName;
    DeclType declarationType;
    std::unique_ptr<Expr> arraySize;
};

class VarDeclStmt : public Stmt {
public:
    VarDeclStmt() = delete;
    explicit VarDeclStmt(std::unique_ptr<Stmt> t_i, const std::string& t_identName)
        : varTypeStmt{std::move(t_i)}, identifierName{t_identName} {}

    std::unique_ptr<Stmt> varTypeStmt;
    std::string identifierName;
};

class VarInitStmt : public Stmt {
public:
    VarInitStmt() = delete;
    explicit VarInitStmt(std::unique_ptr<Stmt> t_varDecl, std::unique_ptr<Expr> t_initializer)
        : varDecl{std::move(t_varDecl)}, initializer{std::move(t_initializer)} {}


    std::unique_ptr<Stmt> varDecl;
    std::unique_ptr<Expr> initializer;
};

class VarAssignStmt : public Stmt {
public:
    VarAssignStmt() = delete;
    explicit VarAssignStmt(const std::string& t_name, const std::string& t_op, std::unique_ptr<Expr> t_assignment)
        : name{t_name}, op{t_op}, assignment{std::move(t_assignment)} {}

    std::string name;
    std::string op;
    std::unique_ptr<Expr> assignment;
};

class ReturnStmt : public Stmt {
public:
    ReturnStmt() = delete;
    explicit ReturnStmt(std::unique_ptr<Expr> t_returnExpr): returnExpr{std::move(t_returnExpr)} {}

    std::unique_ptr<Expr> returnExpr;
};

class BlockStmt : public Stmt {
public:
    BlockStmt() = delete;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> t_stmts): statements{std::move(t_stmts)} {}

    std::vector<std::unique_ptr<Stmt>> statements;
};

struct Parameter {
    std::string type;
    std::string name;
};

class FnDeclStmt : public Stmt {
public:
    FnDeclStmt() = delete;
    explicit FnDeclStmt(const std::string& t_fnName, std::vector<std::unique_ptr<Stmt>> t_parameters, const std::string& t_returnType, std::unique_ptr<Stmt> t_body)
        : functionName{t_fnName}, parameters{std::move(t_parameters)}, returnType{t_returnType}, body{std::move(t_body)} {}

    std::string functionName;
    std::vector<std::unique_ptr<Stmt>> parameters;
    std::string returnType;
    std::unique_ptr<Stmt> body;
};

class IfStmt : public Stmt {
public:
    IfStmt() = delete;
    explicit IfStmt(std::unique_ptr<Stmt> t_init, std::unique_ptr<BoolExpr> t_cond, std::unique_ptr<BlockStmt> t_body)
        : initializer{std::move(t_init)}, condition{std::move(t_cond)}, body{std::move(t_body)} {}

    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<BoolExpr> condition;
    std::unique_ptr<BlockStmt> body;
};

class CondStmt : public Stmt {
public:
    CondStmt() = default;
    void addIfStmt(std::unique_ptr<IfStmt> t_ifStmt) { ifStmt = std::move(t_ifStmt); }
    void addElifStmt(std::unique_ptr<IfStmt> t_ifStmt) { elifStmts.emplace_back(std::move(t_ifStmt)); }
    void addElseBlock(std::unique_ptr<BlockStmt> t_blockStmt) { elseBody = std::move(t_blockStmt); }

    std::unique_ptr<IfStmt> ifStmt;
    std::vector<std::unique_ptr<IfStmt>> elifStmts;
    std::unique_ptr<BlockStmt> elseBody;
};

class ForStmt : public Stmt {
public:
    ForStmt() = delete;
    explicit ForStmt(std::unique_ptr<Stmt> t_init, std::unique_ptr<BoolExpr> t_cond, std::unique_ptr<Stmt> t_inc, std::unique_ptr<Stmt> t_block)
        : initializer{std::move(t_init)}, condition{std::move(t_cond)}, assign{std::move(t_inc)}, block{std::move(t_block)} {}

    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<BoolExpr> condition;
    std::unique_ptr<Stmt> assign;
    std::unique_ptr<Stmt> block;
};

class ForRangeStmt : public Stmt {
public:
    ForRangeStmt() = delete;
    explicit ForRangeStmt(std::unique_ptr<Stmt> t_init, std::unique_ptr<Expr> t_startExpr, std::unique_ptr<Expr> t_endExpr, std::unique_ptr<Stmt> t_block)
        : varDecl{std::move(t_init)}, startExpression{std::move(t_startExpr)}, endExpression{std::move(t_endExpr)}, block{std::move(t_block)} {}

    std::unique_ptr<Stmt> varDecl;
    std::unique_ptr<Expr> startExpression;
    std::unique_ptr<Expr> endExpression;
    std::unique_ptr<Stmt> block;
};

class Program {
public:
    Program() = default;
    std::vector<FnDeclStmt> functions;
};

class Consumer {
public:
    Consumer() = delete;
    explicit Consumer(const std::vector<Token>& t_tokens): tokens_{t_tokens}, index_{0} {}
    [[nodiscard]] bool checkType(TokenType) const;
    [[nodiscard]] bool checkSubType(TokenSubType) const;

    [[nodiscard]] bool matchType(TokenType);
    [[nodiscard]] bool matchSubType(TokenSubType);

    const Token& consumeType(TokenType);
    const Token& consumeSubType(TokenSubType);

    [[nodiscard]] bool isEnd() const;
private:
    const std::vector<Token>& tokens_;
    size_t index_;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>&);
    ~Parser() noexcept = default;
    Program parse();
private:
    Consumer c;
    FnDeclStmt parseFunction();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<BoolExpr> parseBoolExpression();

    std::unique_ptr<Stmt> parseVarTypeStmt();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseVar();
    std::unique_ptr<Stmt> parseVarAssign();

    std::unique_ptr<ForStmt> parseForStmt();
    std::unique_ptr<IfStmt> parseIfStmt();
    std::unique_ptr<CondStmt> parseCondition();
};