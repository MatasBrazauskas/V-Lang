#pragma once

#include "Parser.hpp"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

using namespace std::string_view_literals;

constexpr auto StdInt = "stdint.h"sv;
constexpr auto StdBool = "stdbool.h"sv;

class TypeConverter {
public:
    TypeConverter();
    ~TypeConverter() noexcept = default;
    std::optional<std::string> getType(const std::string& t_type) const;

    mutable std::unordered_set<std::string_view> includeLibraries_;
    std::unordered_map<std::string, std::string> dataTypes_;
};

class CodeGeneration {
public:
    CodeGeneration() = delete;
    explicit CodeGeneration(const std::string&);
    ~CodeGeneration() noexcept = default;
    void generateOutputFile(const Program&);
private:
    void emitFunction(const Function&);
    void emitBlock(const BlockStmt&);
    void emitStmt(const Stmt&);
    std::string emitExpr(const Expr& expr);
    void emitLibraries();

    std::ofstream outFile;
    TypeConverter typeConverter_;
};