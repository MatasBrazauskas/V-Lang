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
    std::unordered_set<std::string> globalVariables_;
};