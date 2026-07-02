#include <fstream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "CodeGeneration.hpp"
#include "StaticCheck.hpp"

int main() {
    std::ifstream file{"temp.v"};
    Lexer lexer;

    for (std::string line; std::getline(file, line);){
        lexer.tokenize(line);
    }


    /*Parser parser{lexer.tokens};
    const auto ast = parser.parse();*/
}
