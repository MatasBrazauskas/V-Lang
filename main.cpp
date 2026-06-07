#include <fstream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "CodeGeneration.hpp"
#include "StaticCheck.hpp"

int main() {
    std::ifstream file{"temp.v"};
    std::vector<std::string> lines;

    for (std::string line; std::getline(file, line);){
        lines.push_back(line);
    }

    Lexer lexer;
    lexer.tokenize(lines);

    Parser parser{lexer.tokens};
    const auto ast = parser.parse();

    StaticCheck staticCheck;
    staticCheck.staticAnalysis(ast);

    CodeGeneration codeGeneration{"temp.c"};
    codeGeneration.generateOutputFile(ast);
}
