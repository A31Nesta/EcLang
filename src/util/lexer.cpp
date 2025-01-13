#include "lexer.hpp"
#include "classes/language.hpp"
#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace eclang::lexer {
    // Based on this guide:
    // https://craftinginterpreters.com/scanning.html

    // PRIVATE
    // ------
    Language* language;
    std::string source;
    size_t lexemeStart;
    size_t charReading;
    size_t line;
    size_t column;

    bool isEOF() {
        return charReading >= source.length();
    }
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
                c == '_';
    }
    bool isAlphanumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }
    char peekLast() {
        return source.at(charReading-1);
    }
    char peek() {
        if (isEOF()) return '\0';
        return source.at(charReading);
    }
    char peekNext() {
        if (charReading+1 >= source.length()) return '\0';
        return source.at(charReading+1);
    }
    char advance() {
        column++; // advance column as well
        return source.at(charReading++);
    }

    // Read specific type of multi-character tokens
    // --------------------------------------------
    Token readAlphanumericToken() {
        while (isAlphanumeric(peek())) advance();

        // create token with no line or column, we just want the string and type
        Token t;
        t.string = source.substr(lexemeStart, charReading - lexemeStart);

        // check for CLASSes
        // We don't check for ATTRIBUTEs as they're context dependent.
        // The ATTRIBUTE Type is unused
        std::vector<std::string> classes = language->getClasses();
        for (std::string className : classes) {
            if (t.string == className) {
                // We now know it's a class so we return this
                t.type = type::CLASS;
                return t;
            }
        }
        // Ok... it's an identifier
        t.type = type::IDENTIFIER;
        return t;
    }
    Token readKeyword() {
        advance(); // This ensures that we take the '#' even if it's not alphanumeric
        // We also allow the usage of '-' because kebab case is allowed in keywords
        while (isAlphanumeric(peek()) || peek() == '-') advance();

        // create token with no line or column, we just want the string and type
        Token t;
        t.string = source.substr(lexemeStart, charReading - lexemeStart);

        // check for KEYWORDs
        const std::array<std::string, ECLANG_KEYWORD_COUNT> keywords = Language::getKeywords();
        for (std::string keyword : keywords) {
            if (t.string == keyword) {
                t.type = type::KEYWORD;
                return t;
            }
        }
        // whoops, invalid keyword
        t.type = type::INVALID;
        return t;
    }
    Token readNumericToken() {
        while (isDigit(peek())) advance();
        // If our next character is a dot and there are more numbers afterwards we take it and continue reading
        if (peek() == '.' && isDigit(peekNext())) {
            advance();
            while (isDigit(peek())) advance();
        }
        // We should now have a token so we return it
        Token t;
        t.string = source.substr(lexemeStart, charReading - lexemeStart);
        t.type = type::NUMBER;
        return t;
    }
    Token readString(char delimiter = '"') {
        // Read everything until we find a delimiter that is not being escaped.
        // We check that either this character is NOT a delimiter or that it IS a delimiter and the previous character is a backslash
        while (!isEOF() && (peek() != delimiter || (peek() == delimiter && peekLast() == '\\'))) {
            // newlines
            if (peek() == '\n') {
                line++;
                column = 0;
            }
            advance();
        }
        // Check that string was properly terminated
        if (isEOF()) {
            std::cerr << "ECLANG_ERROR: Unterminated string starting from column "+std::to_string(column)+" of line "+std::to_string(line)+".\n";
            Token t;
            t.type = type::INVALID;
            t.string = "";
        }
        // closing delimiter
        advance();

        // Save
        Token t;
        t.string = source.substr(lexemeStart + 1, charReading - lexemeStart - 2); // Trim first and last delimiters
        switch (delimiter) {
        case '"':
            t.type = type::STRING;
            break;
        case '`':
            t.type = type::STRING_MD;
            break;
        default:
            // Should never happen because this function should never be called with anything other than " and `
            t.type = type::INVALID;
            break;
        }
        return t;
    }
    bool readComment() {
        // Check for single line comment
        if (!isEOF() && peek() == '/') {
            // Advance until we find a new line
            while (!isEOF() && peek() != '\n') advance();
            return true;
        }
        // If our next character is a dot and there are more numbers afterwards we take it and continue reading
        if (!isEOF() && peek() == '*') {
            // We advance until we find "*/"
            while (!isEOF() && (peek() != '*' || peekNext() != '/')) {
                // newlines
                if (peek() == '\n') {
                    line++;
                    column = 0;
                }
                advance();
            }
            // Check that the comment was properly terminated
            if (isEOF()) {
                std::cerr << "ECLANG_ERROR: Unterminated comment starting from column "+std::to_string(column)+" of line "+std::to_string(line)+".\n";
                return false;
            }
            // advance twice, once for the '*' and another for the '/'.
            advance();advance();
            return true;
        }
        // No single line or multiline comment. return false
        return false;
    }


    Token scanToken() {
        char c = advance();

        // Init token
        Token t;
        t.line = line;
        t.column = column;

        switch (c) {
        // SINGLE CHARACTER TOKENS
        case '{':
            t.type = type::SCOPE_ENTER;
            t.string = c;
            break;
        case '}':
            t.type = type::SCOPE_EXIT;
            t.string = c;
            break;
        case '=':
            t.type = type::ASSIGNMENT;
            t.string = c;
            break;
        case '(':
            t.type = type::PARENTHESIS_OPEN;
            t.string = c;
            break;
        case ')':
            t.type = type::PARENTHESIS_CLOSE;
            t.string = c;
            break;
        case ',':
            t.type = type::COMMA;
            t.string = c;
            break;
        case ';':
            t.type = type::SEMICOLON;
            t.string = c;
            break;

        // WHITE SPACES
        case ' ':
        case '\r':
        case '\t':
            t.type = type::IGNORED;
            t.string = "";
            break;

        // COMMENTS
        case '/': {
            if (readComment()) {
                t.type = type::IGNORED;
                t.string = "";
            } else {
                // If it's not a comment it's a syntactic error.
                std::cerr << "ECLANG_ERROR: Unexpected character '/' in column "+std::to_string(column)+" of line "+std::to_string(line)+". Did you mean to add a comment (// or /*)?\n";
                t.type = type::INVALID;
                t.string = "";
            }
            }break;
        
        // NEW LINE
        // advance line and set column back to 0
        case '\n':
            line++;
            column = 0;
            t.type = type::IGNORED;
            t.string = "";
            break;

        // COMPLEX TOKENS: LITERALS, KEYWORDS, IDENTIFIERS...
        case '"': {
            Token t2 = readString('"');
            t.type = t2.type;
            t.string = t2.string;
            }break;
        case '`': {
            Token t2 = readString('`');
            t.type = t2.type;
            t.string = t2.string;
            }break;

        default:
            // class or identifier
            if (isAlpha(c)) {
                Token t2 = readAlphanumericToken();
                t.type = t2.type;
                t.string = t2.string;
            }
            // keyword
            else if (c == '#') {
                Token t2 = readKeyword();
                t.type = t2.type;
                t.string = t2.string;
                if (t.type == type::INVALID) {
                    std::cerr << "ECLANG_ERROR: Invalid keyword \""+t.string+"\" in column "+std::to_string(column)+" of line "+std::to_string(line)+".\n";
                }
            }
            // Number (with or without symbol)
            else if (isDigit(c) || c == '-' || c == '+') {
                Token t2 = readNumericToken();
                t.type = t2.type;
                t.string = t2.string;
            }
            // Other
            else {
                std::cerr << "ECLANG_ERROR: Unexpected character '"<<c<<"' in column "+std::to_string(column)+" of line "+std::to_string(line)+".\n";
                t.type = type::INVALID;
                t.string = "";
            }
            break;
        }

        return t;
    }

    // FUNCTIONS
    // ---------
    std::vector<Token> tokenizeSource(std::string sourceCode, Language* lang) {
        // Create Vector
        std::vector<Token> tokens;

        bool hadErrors = false;

        // Makes using helper functions a lot easier
        language = lang;
        source = sourceCode;
        lexemeStart = 0;
        charReading = 0;
        line = 2; // Line starts at 2 because we ignore the #language tag
        column = 0;

        // read char by char
        while (!isEOF()) {
            lexemeStart = charReading;
            Token t = scanToken();
            if (t.type == type::INVALID) {
                hadErrors = true;
            } else if (t.type != type::IGNORED) {
                tokens.push_back(t);
            }
        }

        if (hadErrors) {
            throw std::runtime_error("ECLANG_ERROR: Error(s) occured during tokenization. Look for errors above.");
        }
        return tokens;
    }
}