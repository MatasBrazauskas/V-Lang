#include "Lexer.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <cassert>

using namespace std::string_literals;

namespace {
    bool isLetter(const char t_inputChar) {
        if (t_inputChar >= 'a' && t_inputChar <= 'z') {
            return true;
        }

        if (t_inputChar >= 'A' && t_inputChar <= 'Z') {
            return true;
        }
        return false;
    }

    bool isDigit(const char t_inputChar) {
        if (t_inputChar >= '0' && t_inputChar <= '9') {
            return true;
        }

        return false;
    }

    bool isWord(const char t_inputChar) {
        if (isLetter(t_inputChar)) {
            return true;
        }

        if (isDigit(t_inputChar)) {
            return true;
        }

        if (t_inputChar == '_') {
            return true;
        }

        return false;
    }

    bool isOperator(const char t_inputChar) {
        if (t_inputChar == '+' or t_inputChar == '-') {
            return true;
        }

        if (t_inputChar == '*' or t_inputChar == '/') {
            return true;
        }

        if (t_inputChar == '=' or t_inputChar == '%') {
            return true;
        }

        if (t_inputChar == '>' or t_inputChar == '<') {
            return true;
        }

        if (t_inputChar == '!') {
            return true;
        }

        return false;
    }

    bool isBrace(const char t_inputChar) {
        if (t_inputChar == '(' or t_inputChar == ')') {
            return true;
        }

        if (t_inputChar == '{' or t_inputChar == '}') {
            return true;
        }

        if (t_inputChar == '[' or t_inputChar == ']') {
            return true;
        }

        return false;
    }

    bool isSeparator(const char t_inputChar) {
        if (t_inputChar == ',' or t_inputChar == ';') {
            return true;
        }

        if (t_inputChar == ':' or t_inputChar == '.') {
            return true;
        }

        if (t_inputChar == '&' or t_inputChar == '_') {
            return true;
        }

        return false;
    }

    bool isWhiteSpace(const char t_inputChar) {
        return t_inputChar == ' ' || t_inputChar == '\r' || t_inputChar == '\t';
    }

    bool isString(const char t_inputChar) {
        if (t_inputChar == '"') {
            return true;
        }

        return false;
    }

    bool isChar(const char t_inputChar) {
        if (t_inputChar == '\'') {
            return true;
        }

        return false;
    }

    bool isComment(const char t_inputChar, const std::string_view t_line, const int t_index) {
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
}

Token::Token(const TokenType t_type, const TokenSubType t_subType, const std::string_view t_lexeme, const int t_line)
    : type{t_type}, subType{t_subType}, lexeme{t_lexeme}, line{t_line} {}

TokenIdentifier::TokenIdentifier() {
    keywords = {
        {"const", TokenSubType::Const},
        {"fn", TokenSubType::Func},
        {"return", TokenSubType::Return},
        {"if", TokenSubType::If},
        {"elif", TokenSubType::Elif},
        {"else", TokenSubType::Else},
        {"for", TokenSubType::For},
    };

    types = {
        {"int", TokenSubType::Int},
        {"uint", TokenSubType::Uint},
        {"long", TokenSubType::Long},
        {"ulong", TokenSubType::ULong},
        {"float", TokenSubType::Float},
        {"char", TokenSubType::Char},
        {"bool", TokenSubType::Bool},
        {"void", TokenSubType::Void},
    };


    arithmeticOp = {
        {"+", TokenSubType::Plus},
        {"-", TokenSubType::Minus},
        {"*", TokenSubType::Multiply},
        {"/", TokenSubType::Divide},
        {"%", TokenSubType::Module},
    };

    assignOp = {
        {"=", TokenSubType::Assign},
        {"+=", TokenSubType::AssignPlus},
        {"-=", TokenSubType::AssignMinus},
        {"*=", TokenSubType::AssignMultiply},
        {"/=", TokenSubType::AssignDivide},
        {"%=", TokenSubType::AssignModule},
    };

    comparisonOp = {
        {"==", TokenSubType::Equals},
        {"!=", TokenSubType::NotEquals},
        {"<", TokenSubType::Less},
        {">", TokenSubType::More},
        {"<=", TokenSubType::LessEq},
        {">=", TokenSubType::MoreEq},
    };

    logicalOp = {
        {"and", TokenSubType::And},
        {"or", TokenSubType::Or},
        {"not", TokenSubType::Not},
    };

    separators = {
        {"{", TokenSubType::LCurlyBracket},
        {"}", TokenSubType::RCurlyBracket},
        {"(", TokenSubType::LParen},
        {")", TokenSubType::RParen},
        {"[", TokenSubType::LSquareBracket},
        {"]", TokenSubType::RSquareBracket},
        {",", TokenSubType::Comma},
        {";", TokenSubType::Semicolon},
        {":", TokenSubType::Colon},
        {"..", TokenSubType::Range},
        {"&", TokenSubType::Reference},
        {"_", TokenSubType::Default}
    };

    assert(static_cast<size_t>(TokenSubType::EndOfEnum) == keywords.size() + types.size() + arithmeticOp.size() + comparisonOp.size() + logicalOp.size() + separators.size());
}

std::tuple<TokenType, TokenSubType> TokenIdentifier::identifyKeyword(const std::string_view t_keyWord) const {
    if (const auto it = keywords.find(t_keyWord); it != keywords.end()) {
        return std::make_tuple(TokenType::None, it->second);
    }

    if (const auto it = types.find(t_keyWord); it != types.end()) {
        return std::make_tuple(TokenType::Type, it->second);
    }

    if (const auto it = types.find(t_keyWord); it != types.end()) {
        return std::make_tuple(TokenType::Type, it->second);
    }

    if (const auto it = logicalOp.find(t_keyWord); it != logicalOp.end()) {
        return std::make_tuple(TokenType::LogicalOp, it->second);
    }

    return std::make_tuple(TokenType::None, TokenSubType::Identifier);
}

std::tuple<TokenType, TokenSubType> TokenIdentifier::identifyNumberLiteral(const std::string_view t_word) const {
    bool dotPresent{false};
    bool prevUnderscore{false};

    auto type = TokenType::Literal;
    std::string_view suffix;

    for (const char i : t_word) {
        if (i == '.') {
            dotPresent = true;
        } else if (i == '_') {
            if (prevUnderscore) {
                throw std::runtime_error("Trailing underscore ('_').");
            }

            prevUnderscore = true;
        } else if (isDigit(i)) {
            prevUnderscore = false;
        } else if (isLetter(i)) {
            break;
        } else {
            throw std::runtime_error("Invalid character inside of a literal.");
        }
    }

    if (suffix.empty()) {
        throw std::runtime_error("All literals need to have suffixes.");
    }

    std::unordered_map<std::string_view, TokenSubType> map = {
        {"f", TokenSubType::FloatLiteral},
        {"d", TokenSubType::IntLiteral},
        {"u", TokenSubType::UintLiteral},
        {"l", TokenSubType::LongLiteral},
        {"lu", TokenSubType::ULongLiteral},
    };

    const auto it = map.find(t_word);
    if (it == map.end()) {
        throw std::runtime_error("Unknown suffix inside literal.");
    }

    if (dotPresent and it->second != TokenSubType::FloatLiteral) {
        throw std::runtime_error("Expected float literal suffix.");
    }

    if (not dotPresent and it->second != TokenSubType::FloatLiteral) {
        throw std::runtime_error("Expected float literal.");
    }

    return std::make_tuple(type, it->second);
}

TokenParser::TokenParser(): parserState{ParserState::Regular}, rowIndex{0} {}

std::vector<Token> TokenParser::consumeLine(const TokenIdentifier& t_tokenIdent, std::string_view t_line) {
    std::vector<Token> result;

    for (int index = 0; index < t_line.length();) {
        if (parserState == ParserState::StringLiteral) {
            const auto [newIndex, token] = parseString(t_line, index);

            index = newIndex;
            stringLiteral.append(token);

            if (parserState == ParserState::Regular) {
                result.emplace_back(TokenType::Literal, TokenSubType::StringLiteral, stringLiteral, rowIndex);
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

                result.emplace_back(TokenType::Literal, TokenSubType::CharLiteral, token, rowIndex);
            } else if (isLetter(t_line[index])) {
                const auto [newIndex, token] = parseWord(t_line, index);
                const auto [type, subType] = t_tokenIdent.identifyNumberLiteral(token);

                index = newIndex;

                result.emplace_back(type, subType, token, rowIndex);
            } else if (isDigit(t_line[index])) {
                const auto [newIndex, token]= parseNumber(t_line, index);
                const auto [type, subType] = t_tokenIdent.identifyNumberLiteral(token);

                index = newIndex;

                result.emplace_back(type, subType, token, rowIndex);
            } else if (isOperator(t_line[index])) {
                const auto [newIndex, token]= parseOperator(t_line, index);
                const auto [type, subType] = t_tokenIdent.identifyNumberLiteral(token);

                index = newIndex;

                result.emplace_back(type, subType, token, rowIndex);
            } else if (isBrace(t_line[index])) {
                const auto [token, subType] = parseBraces(t_line, index);

                index++;

                result.emplace_back(rowIndex, newIndex, token);
            } else if (isSeparator(t_line[index])) {
                const auto [newIndex, token] = parseSeparator(t_line, index);

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


std::tuple<int, std::string_view> TokenParser::parseWord(std::string_view t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) and isWord(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, token);
}

std::tuple<int, std::string_view> TokenParser::parseNumber(const std::string_view t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) and (isDigit(t_input[index]) or t_input[index] == '_')) {
        ++index;
    }

    if (index == t_input.length() or t_input[index] != '.') {
        const auto token = t_input.substr(t_index, index - t_index);
        return std::make_tuple(index, token);
    }

    const int lastDigitIndex{index};
    index++;

    while (index < static_cast<int>(t_input.length()) and (isDigit(t_input[index]) or t_input[index] == '_')) {
        ++index;
    }

    if (lastDigitIndex + 1 == index) {
        const auto token = t_input.substr(t_index, lastDigitIndex - t_index);
        return std::make_tuple(lastDigitIndex, token);
    }

    const auto token = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, token);
}

std::tuple<int, std::string_view> TokenParser::parseOperator(const std::string_view t_input, const int t_index) const {
    int index{t_index + 1};

    if (index + 1 < static_cast<int>(t_input.length()) and t_input[index] == '=') {
        index++;
    }

    const auto subStr = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, subStr);
}


std::tuple<int, std::string_view> TokenParser::parseBraces(const std::string_view t_input, const int t_index) const {
    const auto strToken = std::string(1, t_input[t_index]);
    const auto newIndex = t_index + 1;

    return std::make_tuple(newIndex, strToken);
}

std::tuple<int, std::string_view> TokenParser::parseSeparator(const std::string_view t_input, int t_index) const {
    int index{t_index};
    while (index < static_cast<int>(t_input.length()) and isSeparator(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);

    return std::make_tuple(index, token);
}

std::tuple<int, std::string_view> TokenParser::parseString(const std::string_view t_input, const int t_index) {
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

std::tuple<int, std::string_view> TokenParser::parseChar(const std::string_view t_input, const int t_index) {
    int index{t_index + 1};
    if (t_input[index] == '\\') {
        index++;
    }

    index++;

    if (t_input[index] != '\'') {
        throw std::runtime_error{"Invalid character `\\'"};
    }

    return {index, t_input.substr(t_index, index - t_index)};
}

int TokenParser::skipIndentation(const std::string_view t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) and isWhiteSpace(t_input[index])) {
        ++index;
    }

    return index;
}

void Lexer::tokenize(std::string_view t_line) {

    if (t_line.empty() || std::ranges::all_of(t_line, isblank)) {
        return;
    }
    const auto strTokens = tokenParser.consumeLine(&tokenIdentifier, t_line);
    tokens.insert(tokens.end(), strTokens.begin(), strTokens.end());
}
