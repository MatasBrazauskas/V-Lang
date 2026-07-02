#include "Lexer.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <cassert>

using namespace std::string_literals;

bool CharIdentifier::isAlphabetic(const char t_inputChar) {
    if (t_inputChar >= 'a' && t_inputChar <= 'z') {
        return true;
    }

    if (t_inputChar >= 'A' && t_inputChar <= 'Z') {
        return true;
    }
    return false;
}

bool CharIdentifier::isDigit(const char t_inputChar) {
    if (t_inputChar >= '0' && t_inputChar <= '9') {
        return true;
    }

    return false;
}

bool CharIdentifier::isWord(const char t_inputChar) {
    if (isAlphabetic(t_inputChar)) {
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

bool CharIdentifier::isOperator(const char t_inputChar) {
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

bool CharIdentifier::isBrace(const char t_inputChar) {
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

bool CharIdentifier::isSeparator(const char t_inputChar) {
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

bool CharIdentifier::isWhiteSpace(const char t_inputChar) {
    return t_inputChar == ' ' || t_inputChar == '\r' || t_inputChar == '\t';
}

bool CharIdentifier::isString(const char t_inputChar) {
    if (t_inputChar == '"') {
        return true;
    }

    return false;
}

bool CharIdentifier::isChar(const char t_inputChar) {
    if (t_inputChar == '\'') {
        return true;
    }

    return false;
}

bool CharIdentifier::isComment(const char t_inputChar, const std::string_view t_line, const int t_index) {
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

TokenIdentifier::TokenIdentifier() {
    keywords= {
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
        {"=", TokenSubType::Assign},
        {"+", TokenSubType::Plus},
        {"-", TokenSubType::Minus},
        {"*", TokenSubType::Multiply},
        {"/", TokenSubType::Divide},
        {"%", TokenSubType::Module},
    };

    assignOp = {
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


std::expected<TokenSubType, std::string> TokenIdentifier::parseNumberLiteral(std::string_view t_word) const {
    bool dotPresent{false};
    bool prevUnderscore{false};
    auto type = TokenSubType::IntLiteral;

    for (const char i : t_word) {
        if (i == '.') {
            if (dotPresent) {
                return std::unexpected("Trailing dot ('.').");
            }

            type = TokenSubType::FloatLiteral;
            dotPresent = true;
        } else if (i == '_') {
            if (prevUnderscore) {
                return std::unexpected("Trailing underscore ('_').");
            }

            prevUnderscore = true;
        } else if (CharIdentifier::isDigit(i)) {
            prevUnderscore = false;
        }
    }

    return type;
}

TokenSubType TokenIdentifier::parseWord(std::string_view t_strToken) const {
    if (const auto it = keywords.find(t_strToken); it != keywords.end()) {
        return it->second;
    }

    if (const auto it = types.find(t_strToken); it != types.end()) {
        return it->second;
    }

    return TokenSubType::Identifier;
}

TokenParser::TokenParser(): parserState{ParserState::Regular}, rowIndex{0} {}

std::vector<Token> TokenParser::consumeLine(const TokenIdentifier& t_tokenIdent, std::string_view t_line) {
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
            if (CharIdentifier::isComment(t_line[index], t_line, index)) {
                index = skipComment(t_line, index);
            } else if (CharIdentifier::isString(t_line[index])) {
                parserState = ParserState::StringLiteral;
            }else if (CharIdentifier::isChar(t_line[index])) {
                const auto [newIndex, token] = parseChar(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isAlphabetic(t_line[index])) {
                const auto [newIndex, token] = generateWord(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isDigit(t_line[index])) {
                const auto [newIndex, token]= parseNumeric(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isOperator(t_line[index])) {
                const auto [newIndex, token]= parseOperator(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isBrace(t_line[index])) {
                const auto [newIndex, token] = parseBraces(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isSeparator(t_line[index])) {
                const auto [newIndex, token] = parseSeparator(t_line, index);

                index = newIndex;
                result.emplace_back(rowIndex, newIndex, token);
            } else if (CharIdentifier::isWhiteSpace(t_line[index])) {
                index = skipIndentation(t_line, index);
            } else {
                throw std::runtime_error("Lexer::tokenize() error");
            }
        }
    }

    rowIndex++;

    return result;
}


std::tuple<int, std::string_view> TokenParser::generateWord(std::string_view t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) && CharIdentifier::isWord(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, token);
}

std::tuple<int, std::string_view> TokenParser::generateNumber(std::string_view t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) and (CharIdentifier::isDigit(t_input[index]) or t_input[index] == '_')) {
        ++index;
    }

    if (index == t_input.length() or t_input[index] != '.') {
        const auto token = t_input.substr(t_index, index - t_index);
        return std::make_tuple(index, token);
    }

    const int lastDigitIndex{index};
    index++;

    while (index < static_cast<int>(t_input.length()) and (CharIdentifier::isDigit(t_input[index]) or t_input[index] == '_')) {
        ++index;
    }

    if (lastDigitIndex + 1 == index) {
        const auto token = t_input.substr(t_index, lastDigitIndex - t_index);
        return std::make_tuple(lastDigitIndex, token);
    }

    const auto token = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, token);
}

std::tuple<int, std::string> TokenParser::parseOperator(const std::string& t_input, const int t_index) const {
    int index{t_index + 1};

    if (index + 1 < static_cast<int>(t_input.length()) and t_input[index] == '=') {
        index++;
    }

    const auto subStr = t_input.substr(t_index, index - t_index);
    return std::make_tuple(index, subStr);
}


std::tuple<int, std::string> TokenParser::parseBraces(const std::string& t_input, const int t_index) const {
    const auto strToken = std::string(1, t_input[t_index]);
    const auto newIndex = t_index + 1;

    return std::make_tuple(newIndex, strToken);
}

std::tuple<int, std::string> TokenParser::parseSeparator(const std::string& t_input, int t_index) const {
    int index{t_index};
    while (index < static_cast<int>(t_input.length()) && CharIdentifier::isSeparator(t_input[index])) {
        ++index;
    }

    const auto token = t_input.substr(t_index, index - t_index);

    return std::make_tuple(index, token);
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
    int index{t_index};
    if (t_input[index] != '\'') {
        throw std::invalid_argument("'");
    }

    index+=2;

    if (t_input[index] != '\'') {
        throw std::invalid_argument("'");
    }

    index++;

    return {index, t_input.substr(t_index, 3)};
}

int TokenParser::skipIndentation(const std::string& t_input, const int t_index) const {
    int index{t_index};

    while (index < static_cast<int>(t_input.length()) && CharIdentifier::isWhiteSpace(t_input[index])) {
        ++index;
    }

    return index;
}
Token::Token(const TokenSubType t_type, const std::string t_lexeme, const int t_line)
    : type{t_type}, lexeme{t_lexeme}, line{t_line} {}

Lexer::Lexer(): tokens{}, tokenParser{}, tokenIdentifier{} {}

void Lexer::tokenize(std::string_view t_line) {

    if (t_line.empty() || std::ranges::all_of(t_line, isblank)) {
        return;
    }
    const auto strTokens = tokenParser.consumeLine(&tokenIdentifier, t_line);
    tokens.insert(tokens.end(), strTokens.begin(), strTokens.end());
}
