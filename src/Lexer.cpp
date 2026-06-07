#include "Lexer.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

static const std::unordered_map<TokenType, std::string> tokenTypeMap = {
    {TokenType::Identifier, "Identifier"},
    {TokenType::Type, "Type"},
    {TokenType::Keyword, "Keyword"},
    {TokenType::Literal, "Literal"},
    {TokenType::Arithmetic, "Arithmetic"},
    {TokenType::Separators, "Separators"},
};

static const std::unordered_map<TokenSubType, std::string> tokenSubTypeMap = {
    {TokenSubType::Identifier, "Identifier"},

    {TokenSubType::IntLiteral, "IntLiteral"},
    {TokenSubType::FloatLiteral, "FloatLiteral"},
    {TokenSubType::StringLiteral, "StringLiteral"},
    {TokenSubType::CharLiteral, "CharLiteral"},

    {TokenSubType::Int, "Int"},
    {TokenSubType::Uint, "Uint"},
    {TokenSubType::Float, "Float"},
    {TokenSubType::Char, "Char"},
    {TokenSubType::Bool, "Bool"},
    {TokenSubType::Void, "Void"},

    {TokenSubType::Func, "Func"},
    {TokenSubType::Return, "Return"},

    {TokenSubType::Assign, "Assign"},
    {TokenSubType::Plus, "Plus"},
    {TokenSubType::Minus, "Minus"},
    {TokenSubType::Multiply, "Multiply"},
    {TokenSubType::Divide, "Divide"},

    {TokenSubType::LBrace, "LBrace"},
    {TokenSubType::RBrace, "RBrace"},
    {TokenSubType::LParen, "LParen"},
    {TokenSubType::RParen, "RParen"},
    {TokenSubType::Comma, "Comma"},
};

TokenParser::TokenParser(): parserState{ParserState::Regular}, rowIndex{0} {}

std::vector<std::tuple<int, int, std::string>> TokenParser::consumeLine(const std::string& t_line) {
    std::vector<std::tuple<int, int, std::string>> result;

    for (int index = 0; index < t_line.length();) {
        if (parserState == ParserState::StringLiteral) {
            const auto [newIndex, token] = parseString(t_line, index);

            index = newIndex;
            stringLiteral.append(token);

            if (parserState == ParserState::Regular) {
                result.emplace_back(rowIndex, newIndex, stringLiteral);
                stringLiteral.clear();
            }
        }
        else {
            if (isComment(t_line[index], t_line, index)) {
                index = skipComment(t_line, index);
            } else if (isString(t_line[index])) {
                parserState = ParserState::StringLiteral;
            }else if (isChar(t_line[index])) {
                const auto [newIndex, token] = parseChar(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (isAlphabetic(t_line[index])) {
                const auto [newIndex, token] = parseWord(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (isNumeric(t_line[index])) {
                const auto [newIndex, token]= parseNumeric(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (isOperator(t_line[index])) {
                const auto [newIndex, token]= parseOperator(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (isBrace(t_line[index])) {
                const auto [newIndex, token] = parseBraces(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (isWhiteSpace(t_line[index])) {
                index = skipIndentation(t_line, index);
            } else {
                throw std::runtime_error("Lexer::tokenize() error");
            }
        }
    }

    rowIndex++;

    return result;
}


bool TokenParser::isAlphabetic(const char t_inputChar) const {
    if (t_inputChar >= 'a' && t_inputChar <= 'z') {
        return true;
    }

    if (t_inputChar >= 'A' && t_inputChar <= 'Z') {
        return true;
    }
    return false;
}

bool TokenParser::isDigit(const char t_inputChar) const {
    if (t_inputChar >= '0' && t_inputChar <= '9') {
        return true;
    }

    return false;
}

bool TokenParser::isNumeric(const char t_inputChar) const {
    if (isDigit(t_inputChar)) {
        return true;
    }

    if (t_inputChar == '.' || t_inputChar == '_') {
        return true;
    }

    return false;
}

bool TokenParser::isAlphaNumeric(const char t_inputChar) const {
    if (isAlphabetic(t_inputChar)) {
        return true;
    }

    if (isDigit(t_inputChar)) {
        return true;
    }

    return false;
}

bool TokenParser::isWord(const char t_inputChar) const {
    if (isAlphaNumeric(t_inputChar)) {
        return true;
    }

    if (t_inputChar == '_') {
        return true;
    }

    return false;
}

bool TokenParser::isOperator(const char t_inputChar) const {
    if (t_inputChar == '+' || t_inputChar == '-') {
        return true;
    }

    if (t_inputChar == '*' || t_inputChar == '/') {
        return true;
    }

    if (t_inputChar == '=') {
        return true;
    }

    return false;
}

bool TokenParser::isBrace(const char t_inputChar) const {
    if (t_inputChar == '(' || t_inputChar == ')') {
        return true;
    }

    if (t_inputChar == '{' || t_inputChar == '}') {
        return true;
    }

    if (t_inputChar == '[' || t_inputChar == ']') {
        return true;
    }

    if (t_inputChar == ',') {
        return true;
    }

    return false;
}

bool TokenParser::isString(const char t_inputChar) const {
    if (t_inputChar == '"') {
        return true;
    }

    return false;
}

bool TokenParser::isChar(const char t_inputChar) const {
    if (t_inputChar == '\'') {
        return true;
    }

    return false;
}

bool TokenParser::isComment(const char t_inputChar, const std::string& t_line, const int t_index) const {
    if (t_inputChar != '/') {
        return false;
    }

    const int index{t_index + 1};
    if (index == t_line.length()) {
        return false;
    }

    if (t_line[index] == '/') {
        return true;
    }

    return false;
}

bool TokenParser::isWhiteSpace(const char t_inputChar) const {
    return t_inputChar == ' ' || t_inputChar == '\r' || t_inputChar == '\t';
}

std::tuple<int, std::string> TokenParser::parseWord(const std::string& t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) && isWord(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);

    return std::make_tuple(index, token);
}

std::tuple<int, std::string> TokenParser::parseNumeric(const std::string& t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) && isNumeric(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);

    return std::make_tuple(index, token);
}

std::tuple<int, std::string> TokenParser::parseOperator(const std::string& t_input, const int t_index) const {
    const auto subStr = t_input.substr(t_index, 1);

    return std::make_tuple(t_index + 2, subStr);
}


std::tuple<int, std::string> TokenParser::parseBraces(const std::string& t_input, const int t_index) const {
    const auto strToken = std::string(1, t_input[t_index]);
    const auto newIndex = t_index + 1;

    return std::make_tuple(newIndex, strToken);
}

int TokenParser::skipComment(const std::string& t_input, const int t_index) {
    return static_cast<int>(t_input.size());
}

std::tuple<int, std::string> TokenParser::parseString(const std::string& t_input, const int t_index) {
    int index = t_index + 1;
    char prevChar = 0;

    const auto insideStr = [](char currChar, char prevChar) {
        return !(currChar == '"' && prevChar != '\\');
    };

    while (index < static_cast<int>(t_input.length()) && insideStr(t_input[index], prevChar)) {
        prevChar = t_input[index];
        ++index;
    }

    if (index == static_cast<int>(t_input.length())) {
        parserState = ParserState::StringLiteral;
        return {index, t_input.substr(t_index, index - t_index)};
    }

    ++index;

    parserState = ParserState::Regular;
    return {index, t_input.substr(t_index, index - t_index)};
}

std::tuple<int, std::string> TokenParser::parseChar(const std::string& t_input, const int t_index) {
    return {t_index + 4, t_input.substr(t_index, 3)};
}

int TokenParser::skipIndentation(const std::string& t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) && isWhiteSpace(t_input[index])) {
        ++index;
    }

    return index;
}
Token::Token(const TokenType t_type, const TokenSubType t_subType, const std::string t_lexeme, const int t_line)
    : type{t_type}, subType{t_subType}, lexeme{t_lexeme}, line{t_line} {}

TokenIdentifier::TokenIdentifier() {
    keywords= {
        {"fn", TokenSubType::Func},
        {"return", TokenSubType::Return},
    };

    types = {
        {"int", TokenSubType::Int},
        {"uint", TokenSubType::Uint},
        {"float", TokenSubType::Float},
        {"char", TokenSubType::Char},
        {"bool", TokenSubType::Bool},
        {"void", TokenSubType::Void},
    };

    arithmetic = {
        {"=", TokenSubType::Assign},
        {"+", TokenSubType::Plus},
        {"-", TokenSubType::Minus},
        {"*", TokenSubType::Multiply},
        {"/", TokenSubType::Divide},
    };
    separators = {
        {"{", TokenSubType::LBrace},
        {"}", TokenSubType::RBrace},
        {"(", TokenSubType::LParen},
        {")", TokenSubType::RParen},
        {",", TokenSubType::Comma},
    };
}

std::vector<Token> TokenIdentifier::consumeStrToken(const TokenParser& t_tokenParser, const std::vector<std::tuple<int, int, std::string>>& t_strTokens) const {
    std::vector<Token> tokens;
    tokens.reserve( t_strTokens.size() + 1);

    for (const auto& [row, col, strToken] : t_strTokens) {
        auto type = TokenType::Identifier;
        auto subType = TokenSubType::Identifier;

        if (const auto it = keywords.find(strToken); it != keywords.end()) {
            type = TokenType::Keyword;
            subType = it->second;
        } else if (const auto it = types.find(strToken); it != types.end()) {
            type = TokenType::Type;
            subType = it->second;
        } else if (const auto it = arithmetic.find(strToken); it != arithmetic.end()) {
            type = TokenType::Arithmetic;
            subType = it->second;
        } else if (const auto it = separators.find(strToken); it != separators.end()) {
            type = TokenType::Separators;
            subType = it->second;
        } else if (t_tokenParser.isDigit(strToken.at(0))) {
            type = TokenType::Literal;
            subType = getNumberType(t_tokenParser, strToken);
        } else if (t_tokenParser.isString(strToken.at(0))) {
            type = TokenType::Literal;
            subType = TokenSubType::StringLiteral;
        } else if (t_tokenParser.isChar(strToken.at(0))) {
            type = TokenType::Literal;
            subType = TokenSubType::CharLiteral;
        }

        tokens.emplace_back(type, subType, strToken, row);
    }

    return tokens;
}

TokenSubType TokenIdentifier::getNumberType(const TokenParser& t_tokenParser, const std::string& t_input) const {
    bool dotPresent{false};
    bool prevUnderscore{false};
    auto type = TokenSubType::IntLiteral;

    for (const char i : t_input) {
        if (t_tokenParser.isDigit(i)) {
            prevUnderscore = false;
        }
        else {
            if (i == '.') {
                if (dotPresent) {
                    throw std::runtime_error("Lexer::tokenize() error");
                }

                type = TokenSubType::FloatLiteral;
                dotPresent = true;
            } else if (i == '_') {
                if (prevUnderscore) {
                    throw std::runtime_error("Lexer::tokenize() error");
                }

                prevUnderscore = true;
            }
        }
    }

    return type;
}

void Lexer::tokenize(const std::vector<std::string>& t_lines) {

    for (const auto& t_line: t_lines) {
        if (t_line.empty() || std::ranges::all_of(t_line, isblank)) {
            continue;
        }
        const auto strTokens = tokenParser.consumeLine(t_line);

        const auto tokensVec = tokenIdentifier.consumeStrToken(tokenParser, strTokens);
        tokens.insert(tokens.end(), tokensVec.begin(), tokensVec.end());
    }

    for (const auto& token : tokens) {
        const std::string type = tokenTypeMap.find(token.type)->second;
        const std::string subType = tokenSubTypeMap.find(token.subType)->second;

        std::cout << token.lexeme << "|" << type << "|" << subType << "|" << token.line << std::endl;
    }
}
