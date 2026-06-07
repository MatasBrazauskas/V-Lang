#include "StaticCheck.hpp"

void StaticCheck::staticAnalysis(const Program& t_program) {
    checkFunctionNames(t_program.functions);
    checkFunctionVariableScopes(t_program.functions);
}

void StaticCheck::checkFunctionNames(const std::vector<Function>& t_functions) const {
    bool mainFunctionPresent{false};
    std::unordered_set<std::string> functionNames;

    for (const auto& function: t_functions) {
        const auto& functionName = function.functionName;

        if (functionNames.contains(functionName)) {
            throw std::runtime_error("Function " + functionName + " already exists");
        }

        functionNames.insert(functionName);

        if (functionName == "main") {
            if (mainFunctionPresent) {
                throw std::runtime_error("Duplicate main function declaration.");
            }
            mainFunctionPresent = true;
        }
    }

    if (not mainFunctionPresent) {
        throw std::runtime_error("No main function declaration found.");
    }
}

void StaticCheck::checkFunctionVariableScopes(const std::vector<Function>& t_functions) const {
    for (const auto& function: t_functions) {
        std::unordered_set<std::string> localScopeVariables;

        for (const auto& [type, name] : function.parameters) {
            if (localScopeVariables.contains(name)) {
                throw std::runtime_error("Argument " + name + " already exists");
            }
            localScopeVariables.insert(name);
        }
    }
}
