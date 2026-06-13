#include "CodeGeneration.hpp"

#include <format>
#include <iostream>

static int indentationLevel_;
static std::string indent() {
    return std::string(indentationLevel_ * 4, ' ');
}

TypeConverter::TypeConverter(): includeLibraries_{} {
    dataTypes_ = {
        {"int", "int32_t"},
        {"uint", "uint32_t"},
        {"float", "float"},
        {"char", "char"},
        {"bool", "bool"},
        {"void", "void"}
    };

    includeLibraries_ = {
        StdInt, StdBool
    };
}

std::optional<std::string> TypeConverter::getType(const std::string& t_type) const {
    static const auto addLibraries = [&](const std::string& type) {
        if (type == "int" || type == "uint") {
            includeLibraries_.insert(StdInt);
        } else if (type == "bool") {
            includeLibraries_.insert(StdBool);
        }
    };

    if (const auto it = dataTypes_.find(t_type); it != dataTypes_.end()) {
        addLibraries(t_type);
        return it->second;
    }
    return std::nullopt;
}

CodeGeneration::CodeGeneration(const std::string& t_fileName): outFile{t_fileName, std::fstream::out | std::fstream::trunc}, typeConverter_{} {}

void CodeGeneration::generateOutputFile(const Program& t_program) {
    emitLibraries();

    for (const auto& fn : t_program.functions) {
        emitFunction(fn);
    }

}

void CodeGeneration::emitFunction(const Function& t_fn) {
    const auto fnTypeOpt = typeConverter_.getType(t_fn.returnType);

    if (fnTypeOpt == std::nullopt) {
        throw std::runtime_error("Bad fn type");
    }

    outFile << indent() << fnTypeOpt.value() << " " << t_fn.functionName << "(";

    for (int i = 0; i < t_fn.parameters.size(); ++i) {
        const auto& [type, name] = t_fn.parameters[i];
        const auto paramTypeOpt = typeConverter_.getType(type);

        if (paramTypeOpt == std::nullopt) {
            throw std::runtime_error("Bad parameter type");
        }

        outFile << indent() << paramTypeOpt.value() << ' ' << name;

        if (i < t_fn.parameters.size() - 1) {
            outFile << ", ";
        }
    }

    outFile << indent() << ")";

    emitBlock(*t_fn.body);
}

void CodeGeneration::emitBlock(const BlockStmt& t_block) {
    outFile << indent() << " {\n";
    indentationLevel_++;

    for (const auto& stmt : t_block.statements) {
        emitStmt(*stmt);
    }

    indentationLevel_--;
    outFile << indent() << "}\n\n";
}

void CodeGeneration::emitStmt(const Stmt& t_stmt) {
    if (const auto ret = dynamic_cast<const ReturnStmt*>(&t_stmt)) {
        outFile << indent() << "return" << ' ' << emitExpr(*ret->returnExpr) << ";\n";
        return;
    }

    if (const auto var = dynamic_cast<const VarInitializerStmt*>(&t_stmt)) {
        const auto initTypeOpt = typeConverter_.getType(var->typeName);
        if (initTypeOpt == std::nullopt) {
            throw std::runtime_error("Bad initializer type");
        }

        outFile << indent() << initTypeOpt.value() << ' ' << var->name << ' ' << "=" << ' ' << emitExpr(*var->initializer) << ";\n";
        return;
    }

    if (const auto var = dynamic_cast<const VarAssignStmt*>(&t_stmt)) {
        outFile << indent() << var->name << ' ' << "=" << ' ' << emitExpr(*var->initializer) << ";\n";
        return;
    }

    if (const auto fnCall = dynamic_cast<const FnDeclStmt*>(&t_stmt)) {
        outFile << indent() << emitExpr(*fnCall->fnCallExpr) << ";\n";
        return;
    }

    throw std::runtime_error("Unsupported statement.");
}

std::string CodeGeneration::emitExpr(const Expr& t_expr) {
    if (auto literal = dynamic_cast<const LiteralExpr*>(&t_expr)) {
        return literal->literalValue;
    }

    if (auto binary = dynamic_cast<const BinaryExpr*>(&t_expr)) {
        return emitExpr(*binary->left) + ' ' + binary->op + ' ' + emitExpr(*binary->right);
    }

    if (auto fn = dynamic_cast<const CallExpr*>(&t_expr)) {
        std::string fnCall = fn->functionName + "(";

        for (int i = 0; i < fn->args.size(); ++i) {
            fnCall += emitExpr(*fn->args[i]);

            if (i < fn->args.size() - 1) {
                fnCall += ", ";
            } else {
                fnCall += ")";
            }
        }
        return fnCall;
    }

    throw std::runtime_error("Bad expression type");
}

void CodeGeneration::emitLibraries() {
    for (const auto& library : typeConverter_.includeLibraries_) {
        outFile << indent() << "#include <" << library << ">\n";
    }
    outFile << '\n';
}