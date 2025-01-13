#include "classes/eclangClass.hpp"
// eclang
#include "classes/attribute.hpp"
#include "classes/language.hpp"
#include "classes/object.hpp"
#include "classes/vectors.hpp"
#include "util/globalConfig.hpp"
#include "util/lexer.hpp"
#include "util/stringUtils.hpp"
// std
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace eclang {
    // Other
    #ifdef ECLANG_DEBUG
    void debugLexer(std::vector<lexer::Token> tokens) {
        std::cout << "ECLANG_LOG: Debugging lexer...\n";
        for (lexer::Token t : tokens) {
            // Set color depending on the token type
            switch (t.type) {
            case lexer::type::SCOPE_ENTER:
            case lexer::type::SCOPE_EXIT:
                std::cout << "\033[38;5;33m"; // blue
                break;
            case lexer::type::ASSIGN:
                std::cout << "\033[38;5;195m"; // light blue
                break;
            case lexer::type::PARENTHESIS_OPEN:
            case lexer::type::PARENTHESIS_CLOSE:
                std::cout << "\033[38;5;141m"; // violet
                break;
            case lexer::type::COMMA:
                std::cout << "\033[38;5;225m"; // light pink
                break;
            case lexer::type::SEMICOLON:
                std::cout << "\033[38;5;192m"; // light green
                break;
            case lexer::type::KEYWORD:
                std::cout << "\033[38;5;92m"; // darker purple, similar to vscode
                break;
            case lexer::type::CLASS:
                std::cout << "\033[38;5;42m"; // Green, similar to vscode
                break;
            case lexer::type::ATTRIBUTE:
            case lexer::type::IDENTIFIER:
                std::cout << "\033[38;5;45m"; // blue, similar to vscode
                break;
            case lexer::type::NUMBER:
                std::cout << "\033[38;5;230m"; // very light yellow
                break;
            case lexer::type::STRING:
                std::cout << "\033[38;5;215m"; // orange, similar to vscode
                break;
            case lexer::type::STRING_MD:
                std::cout << "\033[38;5;209m"; // more red-ish orange
                break;
            case lexer::type::INVALID:
            case lexer::type::IGNORED:
                std::cout << "\033[48;5;160m"; // RED BACKGROUND!!
                break;
            }

            // Print
            std::cout << t.string;

            // Reset color
            std::cout << "\033[0m ";
        }
        std::cout << "\nECLANG_LOG: Done!\n";
    }
    void debugObjectsRecursive(std::vector<Object*> objects, uint8_t indentationLv = 0) {
        std::string indentationStr = "";
        for (uint8_t i = 0; i < indentationLv; i++) {
            indentationStr += "\t";
        }
        for (Object* o : objects) {
            std::vector<Object*> children = o->getChildren();
            std::cout << 
                indentationStr + "N_CHILDREN: " << children.size() << "\n" <<
                indentationStr + "CLASS_NAME: " << o->getClassName() << "\n" <<
                indentationStr + "OBJCT_NAME: " << o->getName() << "\n";
            
            debugObjectsRecursive(children, indentationLv++);
        }
    }
    #endif

    // Compilation Instructions
    namespace instruction {
        enum Type {
            // keywords
            INCLUDE,
            INCLUDE_DYN,
            TEMPLATE,
            TEMPLATE_DYN,
            REGISTER,
            // object interaction
            OBJECT_CREATE,
            CUSTOM_OBJECT_CREATE,
            ATTRIBUTE_SET,
            CUSTOM_ATTRIBUTE_SET,
            ENTER_SCOPE, // Go to the latest node created. Guaranteed to only occur after creating an Object.
            EXIT_SCOPE, // Go to the parent node of the current one.

            COUNT
        };
    }

    /**
        We assume that this is a number assignation. This function sets the current attribute with the value
        obtained from the text file (object) into the current Object
    */
    size_t parseNumberAssignment(std::vector<lexer::Token> tokens, size_t currentIndex, type::Type attributeType, Object* currentScope) {
        const lexer::Token& t = tokens.at(currentIndex);
        const lexer::Token& value = tokens.at(currentIndex+2); // We skip the equals because we know it's there
        const lexer::Token& semicolon = tokens.at(currentIndex+3);

        if (value.type != lexer::type::NUMBER) {
            throw std::runtime_error("ECLANG_ERROR: Invalid assignment at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Number was expected for Attribute \""+t.string+"\".");
        }
        // Is the next token SEMICOLON?
        if (semicolon.type != lexer::type::SEMICOLON) {
            throw std::runtime_error("ECLANG_ERROR: Invalid token at column "+std::to_string(semicolon.column)+" at line "+std::to_string(semicolon.line)+". Semicolon was expected.");
        }

        // Now, do assignation for every number type
        switch (attributeType) {
        case type::INT8:
            currentScope->_addAttribute(new Attribute(t.string, int8_t(std::stoi(value.string))));
            break;
        case type::INT16:
            currentScope->_addAttribute(new Attribute(t.string, int16_t(std::stoi(value.string))));
            break;
        case type::INT32:
            currentScope->_addAttribute(new Attribute(t.string, int32_t(std::stoi(value.string))));
            break;
        case type::INT64:
            currentScope->_addAttribute(new Attribute(t.string, int64_t(std::stol(value.string))));
            break;
        case type::UINT8:
            currentScope->_addAttribute(new Attribute(t.string, uint8_t(std::stoi(value.string))));
            break;
        case type::UINT16:
            currentScope->_addAttribute(new Attribute(t.string, uint16_t(std::stoi(value.string))));
            break;
        case type::UINT32:
            currentScope->_addAttribute(new Attribute(t.string, uint32_t(std::stoul(value.string))));
            break;
        case type::UINT64:
            currentScope->_addAttribute(new Attribute(t.string, uint64_t(std::stoul(value.string))));
            break;
        case type::FLOAT:
            currentScope->_addAttribute(new Attribute(t.string, std::stof(value.string)));
            break;
        case type::DOUBLE:
            currentScope->_addAttribute(new Attribute(t.string, std::stod(value.string)));
            break;
        default:
            throw std::runtime_error("ECLANG_FATAL: Internal error at `parseNumberAssignment()`");
            break;
        }

        return 3; // We always take three: {ASSIGN, NUMBER, SEMICOLON}
    }
    /**
        We assume that this is a vector assignation. This function sets the current attribute with the value
        obtained from the text file (vector) into the current Object
    */
    size_t parseVectorAssignment(std::vector<lexer::Token> tokens, size_t currentIndex, type::Type attributeType, Object* currentScope) {
        const lexer::Token& t = tokens.at(currentIndex);
        const lexer::Token& value = tokens.at(currentIndex+2); // must be identifier in the case of vectors
        if (value.type != lexer::type::IDENTIFIER) {
            throw std::runtime_error("ECLANG_ERROR: Identifier expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+".");
        }
        uint8_t values = 2; // Amount of values in the vector: 2, 3 or 4
        if (
            attributeType == type::VEC3D ||
            attributeType == type::VEC3F ||
            attributeType == type::VEC3I ||
            attributeType == type::VEC3L
        ) {
            values = 3;
        } else if (
            attributeType == type::VEC4D ||
            attributeType == type::VEC4F ||
            attributeType == type::VEC4I ||
            attributeType == type::VEC4L
        ) {
            values = 4;
        }
        char type = 'f'; // 'f' for FLOAT, 'd' for DOUBLE, 'i' for INT, 'l' for LONG
        if (
            attributeType == type::VEC2D ||
            attributeType == type::VEC3D ||
            attributeType == type::VEC4D
        ) {
            type = 'd';
        } else if (
            attributeType == type::VEC2I ||
            attributeType == type::VEC3I ||
            attributeType == type::VEC4I
        ) {
            type = 'i';
        } else if (
            attributeType == type::VEC2L ||
            attributeType == type::VEC3L ||
            attributeType == type::VEC4L
        ) {
            type = 'l';
        }
        
        // Check tokens depending on amount of values
        if (values == 2) {
            // Take 6 Tokens: {PARENTHESIS_OPEN, NUMBER, COMMA, NUMBER, PARENTHESIS_CLOSE, SEMICOLON}
            const lexer::Token& parOpen = tokens.at(currentIndex+3);
            const lexer::Token& num1 = tokens.at(currentIndex+4);
            const lexer::Token& comma = tokens.at(currentIndex+5);
            const lexer::Token& num2 = tokens.at(currentIndex+6);
            const lexer::Token& parClose = tokens.at(currentIndex+7);
            const lexer::Token& semicolon = tokens.at(currentIndex+8);

            // Check things that are not numbers
            if (parOpen.type != lexer::type::PARENTHESIS_OPEN) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parOpen.column)+" at line "+std::to_string(parOpen.line)+".");}
            if (parClose.type != lexer::type::PARENTHESIS_CLOSE) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parClose.column)+" at line "+std::to_string(parClose.line)+".");}
            if (comma.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma.column)+" at line "+std::to_string(comma.line)+".");}
            if (semicolon.type != lexer::type::SEMICOLON) {throw std::runtime_error("ECLANG_ERROR: Semicolon expected at column "+std::to_string(semicolon.column)+" at line "+std::to_string(semicolon.line)+".");}
            // Check numbers
            if (num1.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num1.column)+" at line "+std::to_string(num1.line)+".");}
            if (num2.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num2.column)+" at line "+std::to_string(num2.line)+".");}

            // Now we know that everything is correct
            // Last checks and add to current object
            if (type == 'f') {
                if (!(value.string == "vec2" || value.string == "vec2f")) {throw std::runtime_error("ECLANG_ERROR: 'vec2' or 'vec2f' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec2f(std::stof(num1.string), std::stof(num2.string))));
            }
            else if (type == 'd') {
                if (value.string != "vec2d") {throw std::runtime_error("ECLANG_ERROR: 'vec2d' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec2d(std::stod(num1.string), std::stod(num2.string))));
            }
            else if (type == 'i') {
                if (value.string != "vec2i") {throw std::runtime_error("ECLANG_ERROR: 'vec2i' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec2i(std::stoi(num1.string), std::stoi(num2.string))));
            }
            else if (type == 'l') {
                if (value.string != "vec2l") {throw std::runtime_error("ECLANG_ERROR: 'vec2l' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec2l(std::stol(num1.string), std::stol(num2.string))));
            }

            return 8; // We took a total of 8 tokens.
        } else if (values == 3) {
            // Take 8 Tokens: {PARENTHESIS_OPEN, NUMBER, COMMA, NUMBER, COMMA, NUMBER, PARENTHESIS_CLOSE, SEMICOLON}
            const lexer::Token& parOpen = tokens.at(currentIndex+3);
            const lexer::Token& num1 = tokens.at(currentIndex+4);
            const lexer::Token& comma1 = tokens.at(currentIndex+5);
            const lexer::Token& num2 = tokens.at(currentIndex+6);
            const lexer::Token& comma2 = tokens.at(currentIndex+7);
            const lexer::Token& num3 = tokens.at(currentIndex+8);
            const lexer::Token& parClose = tokens.at(currentIndex+9);
            const lexer::Token& semicolon = tokens.at(currentIndex+10);

            // Check things that are not numbers
            if (parOpen.type != lexer::type::PARENTHESIS_OPEN) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parOpen.column)+" at line "+std::to_string(parOpen.line)+".");}
            if (parClose.type != lexer::type::PARENTHESIS_CLOSE) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parClose.column)+" at line "+std::to_string(parClose.line)+".");}
            if (comma1.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma1.column)+" at line "+std::to_string(comma1.line)+".");}
            if (comma2.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma2.column)+" at line "+std::to_string(comma2.line)+".");}
            if (semicolon.type != lexer::type::SEMICOLON) {throw std::runtime_error("ECLANG_ERROR: Semicolon expected at column "+std::to_string(semicolon.column)+" at line "+std::to_string(semicolon.line)+".");}
            // Check numbers
            if (num1.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num1.column)+" at line "+std::to_string(num1.line)+".");}
            if (num2.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num2.column)+" at line "+std::to_string(num2.line)+".");}
            if (num3.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num3.column)+" at line "+std::to_string(num3.line)+".");}

            // Now we know that everything is correct
            // Last checks and add to current object
            if (type == 'f') {
                if (!(value.string == "vec3" || value.string == "vec3f")) {throw std::runtime_error("ECLANG_ERROR: 'vec3' or 'vec3f' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec3f(std::stof(num1.string), std::stof(num2.string), std::stof(num3.string))));
            }
            else if (type == 'd') {
                if (value.string != "vec3d") {throw std::runtime_error("ECLANG_ERROR: 'vec3d' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec3d(std::stod(num1.string), std::stod(num2.string), std::stod(num3.string))));
            }
            else if (type == 'i') {
                if (value.string != "vec3i") {throw std::runtime_error("ECLANG_ERROR: 'vec3i' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec3i(std::stoi(num1.string), std::stoi(num2.string), std::stoi(num3.string))));
            }
            else if (type == 'l') {
                if (value.string != "vec3l") {throw std::runtime_error("ECLANG_ERROR: 'vec3l' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec3l(std::stol(num1.string), std::stol(num2.string), std::stol(num3.string))));
            }

            return 10; // We took a total of 10 tokens.
        } else {
            // Take 10 Tokens: {PARENTHESIS_OPEN, NUMBER, COMMA, NUMBER, COMMA, NUMBER, COMMA, NUMBER, PARENTHESIS_CLOSE, SEMICOLON}
            const lexer::Token& parOpen = tokens.at(currentIndex+3);
            const lexer::Token& num1 = tokens.at(currentIndex+4);
            const lexer::Token& comma1 = tokens.at(currentIndex+5);
            const lexer::Token& num2 = tokens.at(currentIndex+6);
            const lexer::Token& comma2 = tokens.at(currentIndex+7);
            const lexer::Token& num3 = tokens.at(currentIndex+8);
            const lexer::Token& comma3 = tokens.at(currentIndex+9);
            const lexer::Token& num4 = tokens.at(currentIndex+10);
            const lexer::Token& parClose = tokens.at(currentIndex+11);
            const lexer::Token& semicolon = tokens.at(currentIndex+12);

            // Check things that are not numbers
            if (parOpen.type != lexer::type::PARENTHESIS_OPEN) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parOpen.column)+" at line "+std::to_string(parOpen.line)+".");}
            if (parClose.type != lexer::type::PARENTHESIS_CLOSE) {throw std::runtime_error("ECLANG_ERROR: Parenthesis expected at column "+std::to_string(parClose.column)+" at line "+std::to_string(parClose.line)+".");}
            if (comma1.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma1.column)+" at line "+std::to_string(comma1.line)+".");}
            if (comma2.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma2.column)+" at line "+std::to_string(comma2.line)+".");}
            if (comma3.type != lexer::type::COMMA) {throw std::runtime_error("ECLANG_ERROR: Comma expected at column "+std::to_string(comma3.column)+" at line "+std::to_string(comma3.line)+".");}
            if (semicolon.type != lexer::type::SEMICOLON) {throw std::runtime_error("ECLANG_ERROR: Semicolon expected at column "+std::to_string(semicolon.column)+" at line "+std::to_string(semicolon.line)+".");}
            // Check numbers
            if (num1.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num1.column)+" at line "+std::to_string(num1.line)+".");}
            if (num2.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num2.column)+" at line "+std::to_string(num2.line)+".");}
            if (num3.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num3.column)+" at line "+std::to_string(num3.line)+".");}
            if (num4.type != lexer::type::NUMBER) {throw std::runtime_error("ECLANG_ERROR: Number expected at column "+std::to_string(num4.column)+" at line "+std::to_string(num4.line)+".");}

            // Now we know that everything is correct
            // Last checks and add to current object
            if (type == 'f') {
                if (!(value.string == "vec4" || value.string == "vec4f")) {throw std::runtime_error("ECLANG_ERROR: 'vec4' or 'vec4f' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec4f(std::stof(num1.string), std::stof(num2.string), std::stof(num3.string), std::stof(num4.string))));
            }
            else if (type == 'd') {
                if (value.string != "vec4d") {throw std::runtime_error("ECLANG_ERROR: 'vec4d' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec4d(std::stod(num1.string), std::stod(num2.string), std::stod(num3.string), std::stod(num4.string))));
            }
            else if (type == 'i') {
                if (value.string != "vec4i") {throw std::runtime_error("ECLANG_ERROR: 'vec4i' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec4i(std::stoi(num1.string), std::stoi(num2.string), std::stoi(num3.string), std::stoi(num4.string))));
            }
            else if (type == 'l') {
                if (value.string != "vec4l") {throw std::runtime_error("ECLANG_ERROR: 'vec4l' expected at column "+std::to_string(value.column)+" at line "+std::to_string(value.line)+". Got "+value.string+" instead.");}
                currentScope->_addAttribute(new Attribute(t.string, vec4l(std::stol(num1.string), std::stol(num2.string), std::stol(num3.string), std::stol(num4.string))));
            }

            return 12; // We took a total of 12 tokens.
        }
    }

    // I don't want to include the lexer functions in the eclangClass header
    /**
        Helper function. Given an array of Tokens, and the current index of the token
        corresponding to an IDENTIFIER, returns the amount of tokens consumed.

        To determine the instructions, this function does the following (T is True, F is False):

        1. Check if it's an attribute
            T: This is an assignation; check:
                1. The type of the Attribute
                2. The type of the next Token (ASSIGNATION)
                3. The type of the next Token (NUMBER, STRING, STRING_MD, IDENTIFIER)
                    - For assigning to vector types, check the following:
                        1. That the tokens after ASSIGNATION are {IDENTIFIER, PARENTHESIS_OPEN, NUMBER, COMMA, NUMBER, COMMA, NUMBER, PARENTHESIS_CLOSE} (number of numbers and commas may vary. vec3 was assumed in the example)
                        2. That the IDENTIFIER is a vector of the type that we expect (Attribute type)
                4. That the last token (after we get all the data) is SEMICOLON.
                5. Instruction should be ATTRIBUTE_SET

            F: This may be a custom attribute.
                1. Check for this structure of tokens: {ASSIGNATION, STRING, SEMICOLON}
                    T: Instruction should be CUSTOM_ATTRIBUTE_SET

                    F: This may be a custom Class.
                        1. Check for this structure of tokens {IDENTIFIER, SEMICOLON} or this one {IDENTIFIER, ENTER_SCOPE}
                            T: Instruction should be CUSTOM_CLASS_SET

                            F: Throw Error (Unexpected identifier)
        
        Returns the amount of tokens consumed.
    */
    size_t parseIdentifier(std::vector<lexer::Token> tokens, size_t currentIndex, Object* currentScope, Language* language) {
        const lexer::Token& t = tokens.at(currentIndex);
        if (t.type != lexer::type::IDENTIFIER) throw std::runtime_error("ECLANG_ERROR: Called parseIdentifier() on a token that was not an identifier at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".");

        // Is this an attribute?
        // Check for attribute of the current class
        std::vector<std::string> attributes = language->getAttributes(currentScope->getClassName());
        bool isAttribute = false;
        for (std::string attribute : attributes) {
            if (attribute == t.string) {
                isAttribute = true;
                break;
            }
        }

        // If this IS an attribute this SHOULD be an assignation; check.
        if (isAttribute) {
            // Get next 2 tokens
            const lexer::Token& assignation = tokens.at(currentIndex+1);
            const lexer::Token& value = tokens.at(currentIndex+2); // can be a literal or an identifier in the case of vectors
            // Check for assignation sign, if not found we have an error.
            if (assignation.type != lexer::type::ASSIGN) {
                throw std::runtime_error("ECLANG_ERROR: Invalid token at column "+std::to_string(assignation.column)+" at line "+std::to_string(assignation.line)+". Equals was expected.");
            }

            // INFO: This could be optimized by using the numerical IDs instead of comparing Strings
            type::Type attributeType = language->getAttributeType(currentScope->getClassName(), t.string);
            switch (attributeType) {
            // NUMBERS
            case type::INT8:
            case type::INT16:
            case type::INT32:
            case type::INT64:
            case type::UINT8:
            case type::UINT16:
            case type::UINT32:
            case type::UINT64:
            case type::FLOAT:
            case type::DOUBLE:
                return parseNumberAssignment(tokens, currentIndex, attributeType, currentScope);
                break;
            // STRINGS
            case type::STRING: {
                if (value.type != lexer::type::STRING) {
                    throw std::runtime_error("ECLANG_ERROR: Invalid assignment at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+". String was expected for Attribute \""+t.string+"\".");
                }
                // Is the next token SEMICOLON?
                const lexer::Token& semicolon = tokens.at(currentIndex+3);
                if (semicolon.type != lexer::type::SEMICOLON) {
                    throw std::runtime_error("ECLANG_ERROR: Invalid token at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+". Semicolon was expected.");
                }
                // Create String attribute
                Attribute* a = new Attribute(t.string, value.string, type::STRING);
                currentScope->_addAttribute(a);
                return 3; // We consumed a total of 3 tokens: ASSIGN, STRING and SEMICOLON
            } break;
            case type::STR_MD: {
                if (value.type != lexer::type::STRING_MD) {
                    throw std::runtime_error("ECLANG_ERROR: Invalid assignment at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+". Markdown String was expected for Attribute \""+t.string+"\".");
                }
                // Is the next token SEMICOLON?
                const lexer::Token& semicolon = tokens.at(currentIndex+3);
                if (semicolon.type != lexer::type::SEMICOLON) {
                    throw std::runtime_error("ECLANG_ERROR: Invalid token at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+". Semicolon was expected.");
                }
                // Create String attribute
                Attribute* a = new Attribute(t.string, value.string, type::STR_MD);
                currentScope->_addAttribute(a);
                return 3; // We consumed a total of 3 tokens: ASSIGN, STRING_MD and SEMICOLON
            } break;
            // VECTORS
            case type::VEC2I:
            case type::VEC3I:
            case type::VEC4I:
            case type::VEC2L:
            case type::VEC3L:
            case type::VEC4L:
            case type::VEC2F:
            case type::VEC3F:
            case type::VEC4F:
            case type::VEC2D:
            case type::VEC3D:
            case type::VEC4D:
                return parseVectorAssignment(tokens, currentIndex, attributeType, currentScope);
                break;
            }

            // If for whatever reason we didn't return earlier we return now.
            return 0;
        }

        // If this is NOT an attribute continue
        return 0;
    }

    // PUBLIC
    // ------

    /**
        Constructs an EcLang object from the path to the file.
        The Language (AUII, NEA, Other...) will be automatically
        detected.
    */
    EcLang::EcLang(std::string filepath) {
        // Read file as binary
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("ECLANG_ERROR: Couldn't load file \""+filepath+"\". No such fie or directory");
        }

        // Find file size
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);

        // Put contents into vector
        char* buffer = new char[length];
        if (!file.read(buffer, length)) {
            delete[] buffer;
            throw std::runtime_error("ECLANG_ERROR: Couldn't copy contents of file into buffer");
        }

        // Close file
        file.close();

        // Get file name without extension
        size_t indexOfLastSlash = filepath.find_last_of('/');
        size_t indexOfLastDot = filepath.find_last_of('.');
        // If there are no slash characters in the name, the file name starts in the 0th position
        if (indexOfLastSlash == std::string::npos) {
            indexOfLastSlash = 0;
        } else {
            // Now we point not to the slash (/), but to the next character.
            // We know that there is a next character since we know that the file exists
            indexOfLastSlash++;
        }
        // If there are no dot characters in the name, the file name ends in the last position of the string
        if (indexOfLastDot == std::string::npos) {
            // We set to length and not length - 1 because substr will read `indexOfLastDot - indexOfLastSlash` characters in the string.
            // For example, if `indexOfLastSlash` is 0, we would read `filepath.length()` characters from the position 0 (the whole string)
            indexOfLastDot = filepath.length();
        }
        // Finally get name without extension
        this->name = filepath.substr(indexOfLastSlash, indexOfLastDot - indexOfLastSlash);

        // Initialize EcLang with the raw file data
        initializeEcLang(buffer, length);

        // Delete file buffer
        delete[] buffer;
    }
    /**
        Constructs an EcLang object from raw data that corresponds to the
        contents of a source or compiled EcLang file.
        The Language (AUII, NEA, Other...) will be automatically
        detected.
    */
    EcLang::EcLang(std::string name, void* data, size_t size) {
        this->name = name;
        initializeEcLang(data, size);
    }
    /**
        Guess what it does
    */
    EcLang::~EcLang() {
        for (Object* o : objects) {
            delete o;
        }
    }
    
    /**
        Saves the compiled/decompiled file.
        If a source file was loaded in the constructor we save a compiled file,
        if a compiled file was loaded instead we save a decompiled source file.
    */
    void EcLang::saveToFile(std::string fileWithoutExtension) {
        // If if's a source file we save the compiled file.
        // If not, we save a decompiled source file
        if (fileWasSource) {
            saveToFileCompiled(fileWithoutExtension);
        } else {
            saveToFileSource(fileWithoutExtension);
        }
    }
    /**
        Saves the compiled EcLang file (compiled at initialization or simply loaded)
    */
    void EcLang::saveToFileCompiled(std::string fileWithoutExtension) {
        if (fileWithoutExtension == "") fileWithoutExtension = name;
        // TODO: Implement save to file compiled
    }
    /**
        Saves the source EcLang file.
        If the file specified in the initialization was a compiled file,
        this method will decompile the file. This decompiled file will not have
        any comments, as they're lost during compilation.
    */
    void EcLang::saveToFileSource(std::string fileWithoutExtension) {
        if (fileWithoutExtension == "") fileWithoutExtension = name;
        // TODO: Implement save to file source
    }

    /**
        Returns the Object objects from the current file as a vector.
        The Objects allow us to access all the data with a simple interface
    */
    std::vector<Object*> EcLang::getAllObjects() {
        return objects;
    }

    // PRIVATE
    // -------

    /**
        For both constructors, a void* of data and a size are obtained
        and passed to this function. This function figures out the type of
        the file (language, binary/source...) and constructs the Object
        objects by parsing the source file or interpreting the binary file.

        Throws a runtime error if the language can't be determined.
    */
    void EcLang::initializeEcLang(void* dataRaw, size_t size) {
        // Pass void* to uint8_t to make working with raw data easier
        uint8_t* data = (uint8_t*)dataRaw;
        // Pass dataRaw to String to make working with source files easier. Only used if actually working with strings
        std::string dataString((char*)dataRaw, size);

        // Get all languages
        std::vector<Language> languages = config::getLanguages();

        // Identify first bytes
        // --------------------
        // If they're equal to any Language's identifier bytes, the file
        // is identified as Compiled and a reference to the Language object will be saved.
        // If not, we check if the first bytes of the file are `#language`; in that case
        // the file will be treated as text (Source) and we will try to find the language
        // specified in our configuration
        bool languageFound = false;
        bool isBinary = true; // set to binary by default; arbitrary
        for (Language& languageChecking : languages) {
            std::vector<uint8_t> identifierBytes = languageChecking.getIdentifierBytes();
            uint8_t numberOfBytes = identifierBytes.size();
            bool found = true;

            // If any of the bytes is not the found in our data,
            // set found to false and break
            for (uint8_t i = 0; i < numberOfBytes; i++) {
                if (data[i] != identifierBytes.at(i)) {
                    found = false;
                    break;
                }
            }

            // If found is still true we set our Language to languageChecking and break
            if (found) {
                language = &languageChecking;
                languageFound = true;
                isBinary = true;
                // Log to console
                #ifdef ECLANG_DEBUG
                    std::cout << "ECLANG_LOG: File identified as a binary file compiled with the language \""+language->getName()+"\".\n";
                #endif
                // break
                break;
            }
        }
        // Check if we found the language; if we didn't, try to read the #language tag
        if (!languageFound) {
            // We should only work with the first line
            size_t newlineIndex = dataString.find_first_of('\n');
            if (newlineIndex == std::string::npos) {
                newlineIndex = dataString.length();
            }
            std::string line = string::trim(dataString.substr(0, newlineIndex));

            // Find first instance of a space (' ')
            size_t spaceIndex = line.find_first_of(' ');
            // If the space character is not found throw error
            if (spaceIndex == std::string::npos) {
                throw std::runtime_error("ECLANG_ERROR: File language couldn't be determined. (space character (' ') was not found)");
            }
            // If the substring before the space is not "#language", throw error
            // INFO: Usage of hard-coded "#language" tag. Maybe we should refactor this to avoid hard-coding
            if (line.substr(0, spaceIndex) != "#language") {
                throw std::runtime_error("ECLANG_ERROR: File language couldn't be determined. (#language tag was not found or is not the first word in the file)");
            }

            // Now we know that the file is a source file: We know that it includes a space somewhere and that
            // it has a #language tag at the beginning of the file. Everything after #language should be the name of the language.
            std::string languageName = line.substr(spaceIndex+1);

            // Finally check if we have this language
            for (Language& languageChecking : languages) {
                if (languageChecking.getName() == languageName) {
                    // Set language to language checking and break
                    language = &languageChecking;
                    languageFound = true;
                    isBinary = false;
                    // log to console
                    #ifdef ECLANG_DEBUG
                        std::cout << "ECLANG_LOG: File identified as a source file written in the language \""+languageName+"\".\n";
                    #endif
                    // break
                    break;
                }
            }

            // If not found throw error
            if (!languageFound) {
                throw std::runtime_error(
                    "ECLANG_ERROR: File identified as a source file written in the language \""+languageName+"\","
                    " but no languages with that name were found."
                );
            }
        }

        // Now that we have all the data that we need, we set
        // some information for this object and construct the file
        fileWasSource = !isBinary;
        // Set either source or compiled depending on the file type
        // Also construct the Object objects
        if (fileWasSource) {
            source = dataString;
            constructFromSource(source);
        } else {
            compiled.assign(data, data + size);
            constructFromBinary(compiled);
        }
    }
    /**
        Constructs all the Object objects by parsing a source file.
    */
    void EcLang::constructFromSource(std::string source) {
        source = source.substr(source.find_first_of('\n')+1); // remove #language tag
        std::vector<lexer::Token> tokens = lexer::tokenizeSource(source, language);
        
        #ifdef ECLANG_DEBUG
        // Print entire lexical analysis
        debugLexer(tokens);
        #endif

        // Parsing:
        // This part is context dependent, each iteration of the loop is an "instruction" or a group of instructions:
        // (assignment and object creation are instructions, object creation followed by entering scope is a group of instructions)
        bool hasErrors = false;
        for (size_t current = 0; current < tokens.size(); current++) {
            const lexer::Token& t = tokens.at(current);
            switch (t.type) {
            case lexer::type::KEYWORD: {
                // INCLUDE
                if (t.string == "#include") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }
                    // TODO: Create child EcLang and append its contents to our contents

                    // Update current
                    current += 1;
                } 
                // INCLUDE-DYN
                else if (t.string == "#include-dyn") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }
                    // TODO: Create child EcLang and append its contents to our contents
                    // THIS VERSION SHOULD STORE THE PATH TO THE FILE! WE CANNOT COMPILE THE FILE DIRECTLY
                }
                // TEMPLATE
                else if (t.string == "#template") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type == lexer::type::STRING) {
                        // TODO: Create child EcLang and append our contents to the specified file's template node

                        // Update current
                        current += 1;
                    }
                    // If there's no string afterwards we set this node as template node
                    else {
                        // TODO: Set as Template Node
                    }
                }
                // TEMPLATE-DYN
                else if (t.string == "#template-dyn") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+t.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }
                    // TODO: Create child EcLang and append our contents to the specified file's template node
                    // THIS VERSION SHOULD STORE THE PATH TO THE FILE! WE CANNOT COMPILE THE FILE DIRECTLY

                    // Update current
                    current += 1;
                }
                // REGISTER
                else if (t.string == "#register") {
                    const lexer::Token& alias = tokens.at(current+1); // This should be a String
                    const lexer::Token& file = tokens.at(current+2); // This should also be a String
                    if (alias.type != lexer::type::STRING) {
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+alias.string+"\" at column "+std::to_string(alias.column)+" at line "+std::to_string(alias.line)+". Usage: #register <alias:Sring> <path:String>\n";
                        break;
                    }
                    if (file.type != lexer::type::STRING) {
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". Usage: #register <alias:Sring> <path:String>\n";
                        break;
                    }
                    // TODO: Add specified file into a map with aliases for files

                    // Update current
                    current += 2;
                }
            }break;
            case lexer::type::CLASS: {
                // CLASS should be followed by IDENTIFIER and optionally SCOPE_ENTER.
                // If SCOPE_ENTER is ommited we should have a SEMICOLON.
                // Either way, we need 2 arguments
                const lexer::Token& identifier = tokens.at(current+1);
                const lexer::Token& terminator = tokens.at(current+2); // again, semicolon or enter scope
                if (identifier.type != lexer::type::IDENTIFIER) {
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+identifier.string+"\" at column "+std::to_string(identifier.column)+" at line "+std::to_string(identifier.line)+". Usage: <Class> <name>; or <Class> <name> {}\n";
                    break;
                }
                if (terminator.type == lexer::type::SEMICOLON) {
                    Object* o = new Object(t.string, identifier.string);
                    // Only add to `objects` if the scope is empty
                    // If not, we add it to the latest object in the scope
                    if (scope.empty()) {
                        objects.push_back(o);
                    } else {
                        scope.at(scope.size()-1)->_addChild(o);
                    }
                }
                else if (terminator.type == lexer::type::SCOPE_ENTER) {
                    Object* o = new Object(t.string, identifier.string);
                    // Only add to `objects` if the scope is empty
                    // If not, we add it to the latest object in the scope
                    if (scope.empty()) {
                        objects.push_back(o);
                    } else {
                        scope.at(scope.size()-1)->_addChild(o);
                    }
                    // Add to scope
                    scope.push_back(o);
                }
                else {
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+terminator.string+"\" at column "+std::to_string(terminator.column)+" at line "+std::to_string(terminator.line)+". Semicolon or curly braces were expected after Node declaration.\n";
                }
                // Update current
                current += 2;
            }break;
            case lexer::type::IDENTIFIER: {
                // This is where things get complex
                if (scope.size() == 0) {
                    std::cerr << "ECLANG_ERROR: Tried to close root at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".\n";
                    break;
                }
                // If we are in a node, pass the current Object in the scope
                current += parseIdentifier(tokens, current, scope.at(scope.size()-1), language);
            } break;
            case lexer::type::SCOPE_EXIT:
                if (scope.size() == 0) {
                    std::cerr << "ECLANG_ERROR: Tried to close root at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".\n";
                    break;
                }
                // if we are in a node, pop back
                scope.pop_back();
                break;
            default:
                // Everything that does not start an instruction goes through here.
                // If something is not starting an instruction then it shouldn't be here.
                hasErrors = true;
                std::cerr << "ECLANG_ERROR: Unexpected token \""+t.string+"\" at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".\n";
                break;
            }
        }

        #ifdef ECLANG_DEBUG
        // Prints the tree and some info
        debugObjectsRecursive(objects);
        #endif
    }
    /**
        Constructs all the Object objects by reading a binary file.
    */
    void EcLang::constructFromBinary(std::vector<uint8_t> compiled) {
        // TODO: Implement reading binary file
    }
}