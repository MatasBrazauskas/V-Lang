#include "Lexer.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <flat_map>

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

namespace {
    enum class ParserState{ Regular, StringLiteral };

    class TokenIdentifier {
    public:
        TokenIdentifier();
        ~TokenIdentifier() noexcept = default;
        [[nodiscard]] std::tuple<TokenType, TokenSubType> identifyKeyword(std::string_view) const;
        [[nodiscard]] std::tuple<TokenType, TokenSubType> identifyNumberLiteral(std::string_view) const;
        [[nodiscard]] std::tuple<TokenType, TokenSubType> identifyOperator(std::string_view) const;
        [[nodiscard]] std::tuple<TokenType, TokenSubType> identifySeperator(std::string_view) const;
    private:
        std::flat_map<std::string_view, TokenSubType> keywords;
        std::flat_map<std::string_view, TokenSubType> types;
        std::flat_map<std::string_view, TokenSubType> arithmeticOp;
        std::flat_map<std::string_view, TokenSubType> assignOp;
        std::flat_map<std::string_view, TokenSubType> comparisonOp;
        std::flat_map<std::string_view, TokenSubType> logicalOp;
        std::flat_map<std::string_view, TokenSubType> separators;

        std::flat_map<std::string_view, TokenSubType> literalSuffixes;
    };

    class TokenParser {
    public:
        TokenParser();
        ~TokenParser() noexcept = default;

        [[nodiscard]] std::vector<Token> consumeLine(const TokenIdentifier&, std::string_view);
    private:

        [[nodiscard]] std::tuple<int, std::string_view> parseWord(std::string_view t_input, int) const;
        [[nodiscard]] std::tuple<int, std::string_view> parseNumber(std::string_view t_input, int) const;
        [[nodiscard]] std::tuple<int, std::string_view> parseOperator(std::string_view t_input, int) const;
        [[nodiscard]] std::tuple<int, std::string_view> parseSeparator(std::string_view t_input, int) const;
        [[nodiscard]] std::tuple<int, std::string_view> parseString(std::string_view t_input, int);
        [[nodiscard]] std::tuple<int, std::string_view> parseChar(std::string_view t_input, int) const;

        [[nodiscard]] int skipComment(std::string_view t_input) const;
        [[nodiscard]] int skipIndentation(std::string_view t_input, int) const;

        ParserState parserState;
        std::string stringLiteral;
        int rowIndex;
    };
}

namespace {
    TokenIdentifier::TokenIdentifier() {
        keywords = {
            {"const", TokenSubType::Const},
            {"fn", TokenSubType::Func},
            {"return", TokenSubType::Return},
            {"if", TokenSubType::If},
            {"elif", TokenSubType::Elif},
            {"else", TokenSubType::Else},
            {"for", TokenSubType::For},
            {"true", TokenSubType::True},
            {"false", TokenSubType::False},
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

        literalSuffixes = {
            {"f", TokenSubType::FloatLiteral},
            {"d", TokenSubType::IntLiteral},
            {"u", TokenSubType::UintLiteral},
            {"l", TokenSubType::LongLiteral},
            {"lu", TokenSubType::ULongLiteral},
        };

        assert(std::to_underlying(TokenSubType::EndOfEnum) == keywords.size() + types.size() + assignOp.size() + arithmeticOp.size() + comparisonOp.size() + logicalOp.size() + separators.size());
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

        std::string_view suffix;

        for (const auto [i, item] : std::views::enumerate(t_word)) {
            if (item == '.') {
                dotPresent = true;
            } else if (item == '_') {
                if (prevUnderscore) {
                    throw std::runtime_error("Trailing underscore ('_').");
                }

                prevUnderscore = true;
            } else if (isDigit(item)) {
                prevUnderscore = false;
            } else if (isLetter(item)) {
                suffix = t_word.substr(i);
                break;
            } else {
                throw std::runtime_error("Invalid character inside of a literal.");
            }
        }

        if (suffix.empty()) {
            throw std::runtime_error("All literals need to have suffixes.");
        }

        const auto it = literalSuffixes.find(suffix);
        if (it == literalSuffixes.end()) {
            throw std::runtime_error("Unknown suffix inside literal.");
        }

        if (dotPresent and it->second == TokenSubType::FloatLiteral) {
            return std::make_tuple(TokenType::Literal, it->second);
        }

        if (not dotPresent and it->second != TokenSubType::FloatLiteral) {
            return std::make_tuple(TokenType::Literal, it->second);
        }

        throw std::runtime_error("Invalid character inside of a literal.");
    }


    std::tuple<TokenType, TokenSubType> TokenIdentifier::identifyOperator(const std::string_view t_word) const {
        if (const auto it = arithmeticOp.find(t_word); it != arithmeticOp.end()) {
            return std::make_tuple(TokenType::ArithmeticOp, it->second);
        }

        if (const auto it = assignOp.find(t_word); it != assignOp.end()) {
            return std::make_tuple(TokenType::AssignOp, it->second);
        }

        if (const auto it = comparisonOp.find(t_word); it != comparisonOp.end()) {
            return std::make_tuple(TokenType::ComparisonOp, it->second);
        }

        throw std::runtime_error("Unknown operator.");
    }

    std::tuple<TokenType, TokenSubType> TokenIdentifier::identifySeperator(const std::string_view t_word) const {
        if (const auto it = separators.find(t_word); it != separators.end()) {
            return std::make_tuple(TokenType::Separator, it->second);
        }

        throw std::runtime_error("Unknown separator.");
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
                    index = skipComment(t_line);
                } else if (isString(t_line[index])) {
                    parserState = ParserState::StringLiteral;
                }else if (isChar(t_line[index])) {
                    const auto [newIndex, token] = parseChar(t_line, index);
                    index = newIndex;

                    result.emplace_back(TokenType::Literal, TokenSubType::CharLiteral, token, rowIndex);
                } else if (isLetter(t_line[index])) {
                    const auto [newIndex, token] = parseWord(t_line, index);
                    const auto [type, subType] = t_tokenIdent.identifyKeyword(token);

                    index = newIndex;

                    result.emplace_back(type, subType, token, rowIndex);
                } else if (isDigit(t_line[index])) {
                    const auto [newIndex, token]= parseNumber(t_line, index);
                    const auto [type, subType] = t_tokenIdent.identifyNumberLiteral(token);

                    index = newIndex;

                    result.emplace_back(type, subType, token, rowIndex);
                } else if (isOperator(t_line[index])) {
                    const auto [newIndex, token]= parseOperator(t_line, index);
                    const auto [type, subType] = t_tokenIdent.identifyOperator(token);

                    index = newIndex;

                    result.emplace_back(type, subType, token, rowIndex);
                } else if (isSeparator(t_line[index])) {
                    const auto [newIndex, token] = parseSeparator(t_line, index);
                    const auto [type, subType] = t_tokenIdent.identifySeperator(token);

                    index = newIndex;

                    result.emplace_back(type, subType, token, rowIndex);
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

        if (index == t_input.length()) {
            const auto token = t_input.substr(t_index);
            return std::make_tuple(index, token);
        }

        if (isLetter(t_input[index])) {
            while (index < static_cast<int>(t_input.length()) and isLetter(t_input[index])) {
                ++index;
            }

            const auto token = t_input.substr(t_index, index - t_index);
            return std::make_tuple(index, token);
        }

        if (t_input[index] != '.') {
            throw std::runtime_error("Expected to get the floating point number.");
        }

        index++;
        const auto lastIndex{index};

        while (index < static_cast<int>(t_input.length()) and (isDigit(t_input[index]) or t_input[index] == '_')) {
            ++index;
        }

        if (lastIndex == index) {
            throw std::runtime_error("Expected numbers after the dot in floating point number.");
        }

        if (index == t_input.length()) {
            const auto token = t_input.substr(t_index, index - t_index);
            return std::make_tuple(index, token);
        }

        if (isLetter(t_input[index])) {
            const auto [i, token] = parseWord(t_input, index);
            return std::make_tuple(i, token);
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

    std::tuple<int, std::string_view> TokenParser::parseSeparator(const std::string_view t_input, const int t_index) const {
        if (t_input[t_index] == '.' and t_input.length() > t_index + 1 and t_input[t_index + 1] == '.') {
            const auto token = t_input.substr(t_index, 2);
            return std::make_tuple(t_index + 2, token);
        }
        const auto token = t_input.substr(t_index, 1);
        return std::make_tuple(t_index + 1, token);
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

    std::tuple<int, std::string_view> TokenParser::parseChar(const std::string_view t_input, const int t_index) const {
        int index{t_index + 1};
        if (t_input[index] == '\\') {
            index++;
        }

        index++;

        if (t_input[index] != '\'') {
            throw std::runtime_error{"Invalid character `\\'"};
        }

        index++;

        return {index, t_input.substr(t_index, index - t_index)};
    }

    int TokenParser::skipComment(const std::string_view t_input) const {
        return t_input.length();
    }

    int TokenParser::skipIndentation(const std::string_view t_input, const int t_index) const {
        const auto it = std::find_if(t_input.begin() + t_index, t_input.end(), isWhiteSpace);
        if (it == t_input.end()) {
            return t_input.length();
        }

        return std::distance(t_input.begin(), it) + 1;
    }
}

struct Lexer::Impl {
    std::vector<Token> tokens;
    TokenParser tokenParser;
    TokenIdentifier tokenIdentifier;
};

Token::Token(const TokenType t_type, const TokenSubType t_subType, const std::string_view t_lexeme, const int t_line)
    : type{t_type}, subType{t_subType}, lexeme{t_lexeme}, line{t_line} {}

Lexer::Lexer(): impl{std::make_unique<Impl>()} {}

Lexer::~Lexer() = default;

void Lexer::tokenize(const std::string_view source) const {
    const auto newTokens = impl->tokenParser.consumeLine(impl->tokenIdentifier, source);

    impl->tokens.insert(impl->tokens.end(), newTokens.begin(),newTokens.end());
}

void Lexer::clear() const {
    impl->tokens.clear();
}

const std::vector<Token>& Lexer::getTokens() const {
    return impl->tokens;
}
