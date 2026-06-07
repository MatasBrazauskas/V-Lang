#pragma once

#include "Parser.hpp"

#include <unordered_set>
#include <string>

class StaticCheck {
public:
    StaticCheck() = default;
    ~StaticCheck() noexcept = default;

    void staticAnalysis(const Program&);
private:
    void checkFunctionNames(const std::vector<Function>&) const;
    void checkFunctionVariableScopes(const std::vector<Function>&) const;
    std::unordered_set<std::string> globalVariables_;
};