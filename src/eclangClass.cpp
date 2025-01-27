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

// Defines (Instructions for clearer compiler code)
#define INST_CREATE         uint8_t(0x01)
#define INST_ATTRIBUTE      uint8_t(0x02)
#define INST_SCOPE_ENTER    uint8_t(0x03)
#define INST_SCOPE_EXIT     uint8_t(0x04)
#define INST_INCLUDE        uint8_t(0x05)
#define INST_TEMPLATE       uint8_t(0x06)
#define INST_MARK_TEMPLATE  uint8_t(0x07)

// When creating an object or attribute during compilation,
// we might find a custom class or attribute. In this case
// we enter INST_ATTR_CUSTOM (255) as an attribute followed
// by a STRING.
#define INST_ATTR_CUSTOM    uint8_t(255)


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
            std::vector<Object*> children = o->getObjects();
            std::cout << 
                indentationStr + "N_CHILDREN: " << children.size() << "\n" <<
                indentationStr + "N_ATTRIBUT: " << o->getAttributes().size() << "\n" <<
                indentationStr + "CLASS_NAME: " << o->getClassName() << "\n" <<
                indentationStr + "OBJCT_NAME: " << o->getName() << "\n\n";
            
            debugObjectsRecursive(children, indentationLv+1);
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

    /**
        Takes a std::string and returns that same string in EcLang's binary format.
    */
    std::vector<uint8_t> compileString(std::string string, bool isMarkdown = false) {
        std::vector<uint8_t> binary;

        binary.push_back(isMarkdown ? 1 : 0); // 0 or 1 depending on whether or not this is a markdown string
        for (char c : string) {
            binary.push_back(uint8_t(c)); // insert all chars
        }
        binary.push_back(0); // NULL-terminate

        return binary;
    }
    /**
        Takes a stream of bytes and an index and reads until the next 0.
        This function will set the index passed to the position after the string
        This string will have a 'n' or a 'm' prefix indicating whether it is
        a Normal String or a Markdown String
    */
    std::string decompileString(std::vector<uint8_t>& binary, size_t& index) {
        // Get the type of String
        char prefix;
        if (binary.at(index) == 0) {
            prefix = 'n';
        } else {
            prefix = 'm';
        }

        std::string decompiled;

        // Get all the chars until we see a 0
        for (size_t i = index+1; i < binary.size(); i++) {
            if (binary.at(i) == 0) {
                index = i;
                break;
            }

            decompiled += binary.at(i);
        }

        return prefix + decompiled;
    }

    /**
        Creates an Attribute object by reading raw binary data.
        This function obtains the value of the attribute by reading the type
        of the data
    */
    Attribute* createAttributeFromBinary(std::vector<uint8_t>& binary, size_t& index, uint8_t classID, Language* language) {
        // Get the Name and Type of the attribute
        std::string attributeName = language->getAttributeName(classID, binary.at(index));
        type::Type type = language->getAttributeType(classID, binary.at(index));
        index++;

        // Get the value
        switch (type) {
        case type::INT8: {
            int8_t value = *reinterpret_cast<int8_t*>(&binary.at(index));
            return new Attribute(attributeName, value);
        }
        case type::INT16: {
            uint8_t bytes[2];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            index += 1;
            int16_t value = *reinterpret_cast<int16_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::INT32: {
            uint8_t bytes[4];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            index += 3;
            int32_t value = *reinterpret_cast<int32_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::INT64: {
            uint8_t bytes[8];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            bytes[4] = binary.at(index+4);
            bytes[5] = binary.at(index+5);
            bytes[6] = binary.at(index+6);
            bytes[7] = binary.at(index+7);
            index += 7;
            int64_t value = *reinterpret_cast<int64_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::UINT8: {
            return new Attribute(attributeName, binary.at(index));
        }
        case type::UINT16: {
            uint8_t bytes[2];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            index += 1;
            uint16_t value = *reinterpret_cast<uint16_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::UINT32: {
            uint8_t bytes[4];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            index += 3;
            uint32_t value = *reinterpret_cast<uint32_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::UINT64: {
            uint8_t bytes[8];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            bytes[4] = binary.at(index+4);
            bytes[5] = binary.at(index+5);
            bytes[6] = binary.at(index+6);
            bytes[7] = binary.at(index+7);
            index += 7;
            uint64_t value = *reinterpret_cast<uint64_t*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::FLOAT: {
            uint8_t bytes[4];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            index += 3;
            float value = *reinterpret_cast<float*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::DOUBLE: {
            uint8_t bytes[8];
            bytes[0] = binary.at(index);
            bytes[1] = binary.at(index+1);
            bytes[2] = binary.at(index+2);
            bytes[3] = binary.at(index+3);
            bytes[4] = binary.at(index+4);
            bytes[5] = binary.at(index+5);
            bytes[6] = binary.at(index+6);
            bytes[7] = binary.at(index+7);
            index += 7;
            double value = *reinterpret_cast<double*>(&bytes);
            return new Attribute(attributeName, value);
        }
        case type::STRING: {
            std::string value = decompileString(binary, index);
            return new Attribute(attributeName, value.substr(1), type::STRING);
        }
        case type::STR_MD: {
            std::string value = decompileString(binary, index);
            return new Attribute(attributeName, value.substr(1), type::STR_MD);
        }
        case type::VEC2I: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec2i));
            index += sizeof(vec2i) - 1;
            vec2i value = *reinterpret_cast<vec2i*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC3I: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec3i));
            index += sizeof(vec3i) - 1;
            vec3i value = *reinterpret_cast<vec3i*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC4I: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec4i));
            index += sizeof(vec4i) - 1;
            vec4i value = *reinterpret_cast<vec4i*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC2L: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec2l));
            index += sizeof(vec2l) - 1;
            vec2l value = *reinterpret_cast<vec2l*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC3L: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec3l));
            index += sizeof(vec3l) - 1;
            vec3l value = *reinterpret_cast<vec3l*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC4L: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec4l));
            index += sizeof(vec4l) - 1;
            vec4l value = *reinterpret_cast<vec4l*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC2F: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec2f));
            index += sizeof(vec2f) - 1;
            vec2f value = *reinterpret_cast<vec2f*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC3F: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec3f));
            index += sizeof(vec3f) - 1;
            vec3f value = *reinterpret_cast<vec3f*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC4F: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec4f));
            index += sizeof(vec4f) - 1;
            vec4f value = *reinterpret_cast<vec4f*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC2D: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec2d));
            index += sizeof(vec2d) - 1;
            vec2d value = *reinterpret_cast<vec2d*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC3D: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec3d));
            index += sizeof(vec3d) - 1;
            vec3d value = *reinterpret_cast<vec3d*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        case type::VEC4D: {
            std::vector<uint8_t> subVec(binary.begin() + index, binary.begin() + index + sizeof(vec4d));
            index += sizeof(vec4d) - 1;
            vec4d value = *reinterpret_cast<vec4d*>(subVec.data());
            return new Attribute(attributeName, value);
        }
        }
        return nullptr;
    }

    // PUBLIC
    // ------

    /**
        Constructs an EcLang object from the path to the file.
        The Language (AUII, NEA, Other...) will be automatically
        detected.
    */
    EcLang::EcLang(std::string filepath, uint8_t fileID) : currentFile(fileID) {
        // The filepath specified may be an alias.
        // This string contains the actual file path that we can load.
        std::string trueFilepath;

        // Get alias if it exists
        if (config::filepathIsAlias(filepath)) {
            trueFilepath = config::filepathGetFor(filepath);
        } else {
            trueFilepath = filepath;
        }

        // Read file as binary
        std::ifstream file(trueFilepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("ECLANG_ERROR: Couldn't load file \""+trueFilepath+"\". No such fie or directory");
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
        size_t indexOfLastSlash = trueFilepath.find_last_of('/');
        size_t indexOfLastDot = trueFilepath.find_last_of('.');
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
            // For example, if `indexOfLastSlash` is 0, we would read `trueFilepath.length()` characters from the position 0 (the whole string)
            indexOfLastDot = trueFilepath.length();
        }
        // Finally get name without extension
        // Register this name into array if this is the original file loaded by the user (fileID = 0)
        // This is used when exporting the file
        if (fileID == 0) {
            includedFilenames.push_back(trueFilepath.substr(indexOfLastSlash, indexOfLastDot - indexOfLastSlash));
        }

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
    EcLang::EcLang(std::string name, void* data, size_t size, uint8_t fileID) : currentFile(fileID) {
        // Register name into array if this is the original file loaded by the user (fileID = 0)
        // This is used when exporting the file
        if (fileID == 0) {
            includedFilenames.push_back(name);
        }
        initializeEcLang(data, size);
    }
    /**
        Guess what it does
    */
    EcLang::~EcLang() {
        // We should only delete our objects if this file was not included by another file
        if (isIncluded) { return; }
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
        // Check: is this file the one loaded by the user?
        if (currentFile != 0) throw std::runtime_error("ECLANG_FATAL: Attempted to compile a dynamically included file");
        // Check: is the name valid?
        if (fileWithoutExtension == "") fileWithoutExtension = includedFilenames[0];
        
        const std::vector<uint8_t> compiledFile = compile();
        const std::string filename = fileWithoutExtension + "." + language->getExtensionCompiled();

        std::fstream outputFile(filename, std::ios::out | std::ios::binary);
        outputFile.write((char*)compiledFile.data(), compiledFile.size());
        outputFile.close();
        #ifdef ECLANG_DEBUG
        std::cout << "ECLANG_LOG: Saved Compiled file to \""+filename+"\"\n";
        #endif
    }
    /**
        Saves the source EcLang file.
        If the file specified in the initialization was a compiled file,
        this method will decompile the file. This decompiled file will not have
        any comments, as they're lost during compilation.
    */
    void EcLang::saveToFileSource(std::string fileWithoutExtension) {
        // Check: is this file the one loaded by the user?
        if (currentFile != 0) throw std::runtime_error("ECLANG_FATAL: Attempted to compile a dynamically included file");
        // Check: is the name valid?
        if (fileWithoutExtension == "") fileWithoutExtension = includedFilenames[0];
        
        const std::string source = decompile();
        const std::string filename = fileWithoutExtension + "." + language->getExtensionSource();

        std::fstream outputFile(filename, std::ios::out);
        outputFile << source;
        outputFile.close();
        #ifdef ECLANG_DEBUG
        std::cout << "ECLANG_LOG: Saved Decompiled file to \""+filename+"\"\n";
        #endif
    }

    /**
        Returns the Object objects from the current file as a vector.
        The Objects allow us to access all the data with a simple interface
    */
    std::vector<Object*> EcLang::getAllObjects() {
        return objects;
    }
    /**
        Returns the Object objects with the class name specified
        from the current file as a vector.
    */
    std::vector<Object*> EcLang::getObjectsByClass(std::string className) {
        std::vector<Object*> objsWithClass;
        for (Object* o : objects) {
            if (o->getClassName() == className) {
                objsWithClass.push_back(o);
            }
        }
        return objsWithClass;
    }
    /**
        Returns the Object object with the name specified.
        The pointer returned may be nullptr.
    */
    Object* EcLang::getObject(std::string name) {
        // Is this a path?
        size_t indexOfSlash = name.find_first_of('/');
        if (indexOfSlash != std::string::npos) {
            std::string firstNode = name.substr(0, indexOfSlash);
            for (Object* o : objects) {
                if (o->getName() == firstNode) {
                    // Get Node by path from Object (This is recursive)
                    return o->getObject(name.substr(indexOfSlash+1));
                }
            }
        } else {
            // This is not a path, just a single node
            for (Object* o : objects) {
                if (o->getName() == name) {
                    return o;
                }
            }
        }
        return nullptr;
    }
    // FOR USE IN ANOTHER ECLANG (Private, moved up here to keep it close to getAllObjects)
    /**
        Returns the Object objects from the current file as a vector.
        This function marks this object as "included", which effectively
        transfers the ownership of the objects to the object calling this
        function (the parent node during inclusion)
    */
    std::vector<Object*> EcLang::_getAllObjectsAsInclude() {
        isIncluded = true;
        return objects;
    }
    /**
        Returns the Template Node Path.
        The template node path will be empty if the file doesn't contain
        a #template tag.
    */
    std::vector<Object*> EcLang::_getTemplateNodePath() {
        return templateNode;
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
        std::vector<Language>& languages = config::getLanguages();

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
            // TODO: Extract bytes from file until we find NULL (0) and compare the sequences instead of comparing directly
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
                // TODO: Disallow including files written in other languages statically (compiler limitation)
                if (t.string == "#include") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }
                    // Create child EcLang and append its contents to our contents
                    #ifdef ECLANG_DEBUG
                    std::cout << "ECLANG_LOG: Statically including file: "+file.string+"\n";
                    #endif
                    EcLang includedEcLang(file.string);
                    std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                    // Add to current object in scope OR simply add to root
                    if (scope.empty()) {
                        objects.insert(objects.end(), children.begin(), children.end());
                    } else {
                        scope.at(scope.size()-1)->_addChildren(children);
                    }

                    // Update current
                    current += 1;
                } 
                // INCLUDE-DYN
                else if (t.string == "#include-dyn") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }
                    // We're including dynamically. If this file is the original file loaded by the user,
                    // we will save the filename and give the new EcLang object its fileID number
                    // If this file is being dynamically included, we include statically and set currentFile
                    // to our currentFile so that it appears as if this file had the new file's nodes
                    uint8_t includedFile = (currentFile==0) ? includedFilenames.size() : currentFile;

                    #ifdef ECLANG_DEBUG 
                    if (currentFile != 0) {
                        std::cout << "ECLANG_LOG: Dynamic inclusion detected in dynamically included file... including statically: "+file.string+"\n";
                    } else {
                        std::cout << "ECLANG_LOG: Dynamically including file: "+file.string+"\n";
                    }
                    #endif

                    EcLang includedEcLang(file.string, includedFile);
                    
                    // We only have to do this is the file included by the user. We register the path
                    // so that the file ID actually points to something lol
                    // The extra "i" indicates that this is an include
                    if (currentFile == 0) {
                        includedFilenames.push_back("i"+file.string);
                    }

                    // Include into our current scene
                    std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                    // Add to current object in scope OR simply add to root
                    if (scope.empty()) {
                        objects.insert(objects.end(), children.begin(), children.end());
                    } else {
                        scope.at(scope.size()-1)->_addChildren(children);
                    }

                    // Update current
                    current += 1;
                }
                // TEMPLATE
                // TODO: Disallow using files written in other languages as templates statically (compiler limitation)
                else if (t.string == "#template") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type == lexer::type::STRING) {
                        // We can't use multiple templates in the same file
                        if (!externalTemplateNode.empty()) {
                            hasErrors = true;
                            std::cerr << "ECLANG_ERROR: Using multiple templates (statically or dynamically) in a single file is not permitted.\n";
                            current += 1;
                            break;
                        }
                        // Create child EcLang and append our contents to the specified file's template node
                        #ifdef ECLANG_DEBUG
                        std::cout << "ECLANG_LOG: Statically including Template file: "+file.string+"\n";
                        #endif
                        EcLang includedEcLang(file.string);
                        std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                        externalTemplateNode = includedEcLang._getTemplateNodePath();
                        // Add to current object in scope OR simply add to root
                        if (scope.empty()) {
                            objects.insert(objects.end(), children.begin(), children.end());
                        } else {
                            scope.at(scope.size()-1)->_addChildren(children);
                        }
                        // Add template node to current scope (even if any of the vectors is empty)
                        scope.insert(scope.end(), externalTemplateNode.begin(), externalTemplateNode.end());

                        // Update current
                        current += 1;
                    }
                    // If there's no string afterwards we set this node as template node
                    else {
                        // Set current scope as Template Node
                        templateNode = scope;
                    }
                }
                // TEMPLATE-DYN
                else if (t.string == "#template-dyn") {
                    const lexer::Token& file = tokens.at(current+1); // This should be a String
                    if (file.type != lexer::type::STRING) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+t.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". String was expected\n";
                        break;
                    }

                    // We can't use multiple templates in the same file
                    if (!externalTemplateNode.empty()) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Using multiple templates (statically or dynamically) in a single file is not permitted.\n";
                        current += 1;
                        break;
                    }
                    
                    // We're including dynamically. If this file is the original file loaded by the user,
                    // we will save the filename and give the new EcLang object its fileID number
                    // If this file is being dynamically included, we include statically and set currentFile
                    // to our currentFile so that it appears as if this file had the new file's nodes
                    uint8_t includedFile = (currentFile==0) ? includedFilenames.size() : currentFile;

                    #ifdef ECLANG_DEBUG 
                    if (currentFile != 0) {
                        std::cout << "ECLANG_LOG: Dynamic inclusion detected in dynamically included Template file... including statically: "+file.string+"\n";
                    } else {
                        std::cout << "ECLANG_LOG: Dynamically including Template file: "+file.string+"\n";
                    }
                    #endif

                    EcLang includedEcLang(file.string, includedFile);
                    
                    // We only have to do this is the file included by the user. We register the path
                    // so that the file ID actually points to something.
                    // The extra "t" indicates that this is a template
                    if (currentFile == 0) {
                        includedFilenames.push_back("t"+file.string);
                    }

                    // Include into our current scene
                    std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                    externalTemplateNode = includedEcLang._getTemplateNodePath();
                    // Add to current object in scope OR simply add to root
                    if (scope.empty()) {
                        objects.insert(objects.end(), children.begin(), children.end());
                    } else {
                        scope.at(scope.size()-1)->_addChildren(children);
                    }
                    // Add template node to current scope (even if any of the vectors is empty)
                    scope.insert(scope.end(), externalTemplateNode.begin(), externalTemplateNode.end());

                    // Update current
                    current += 1;
                }
                // REGISTER
                else if (t.string == "#register") {
                    const lexer::Token& alias = tokens.at(current+1); // This should be a String
                    const lexer::Token& file = tokens.at(current+2); // This should also be a String
                    if (alias.type != lexer::type::STRING) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+alias.string+"\" at column "+std::to_string(alias.column)+" at line "+std::to_string(alias.line)+". Usage: #register <alias:Sring> <path:String>\n";
                        break;
                    }
                    if (file.type != lexer::type::STRING) {
                        hasErrors = true;
                        std::cerr << "ECLANG_ERROR: Unexpected token \""+file.string+"\" at column "+std::to_string(file.column)+" at line "+std::to_string(file.line)+". Usage: #register <alias:Sring> <path:String>\n";
                        break;
                    }
                    // Add specified file to configuration
                    config::filepathRegister(alias.string, file.string);

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
                    hasErrors = true;
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+identifier.string+"\" at column "+std::to_string(identifier.column)+" at line "+std::to_string(identifier.line)+". Usage: <Class> <name>; or <Class> <name> {}\n";
                    break;
                }
                if (terminator.type == lexer::type::SEMICOLON) {
                    Object* o = new Object(t.string, identifier.string, currentFile);
                    // Only add to `objects` if the scope is empty
                    // If not, we add it to the latest object in the scope
                    if (scope.empty()) {
                        objects.push_back(o);
                    } else {
                        scope.at(scope.size()-1)->_addChild(o);
                    }
                }
                else if (terminator.type == lexer::type::SCOPE_ENTER) {
                    Object* o = new Object(t.string, identifier.string, currentFile);
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
                    hasErrors = true;
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+terminator.string+"\" at column "+std::to_string(terminator.column)+" at line "+std::to_string(terminator.line)+". Semicolon or curly braces were expected after Node declaration.\n";
                }
                // Update current
                current += 2;
            }break;
            case lexer::type::IDENTIFIER: {
                // This is where things get complex
                if (scope.size() == 0) {
                    hasErrors = true;
                    std::cerr << "ECLANG_ERROR: Tried to close root at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".\n";
                    break;
                }
                // If we are in a node, pass the current Object in the scope
                current += parseIdentifier(tokens, current, scope.at(scope.size()-1), language);
            } break;
            case lexer::type::SCOPE_EXIT:
                if (scope.size() == 0) {
                    hasErrors = true;
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

        if (hasErrors) {
            throw std::runtime_error("ECLANG_FATAL: Errors found while parsing. Check for errors above.");
        }

        #ifdef ECLANG_DEBUG
        // Prints the tree and some info
        // std::cout << "-------------------------------------------------\n";
        // debugObjectsRecursive(objects);
        // std::cout << "-------------------------------------------------\n";
        #endif
    }
    /**
        Constructs all the Object objects by reading a binary file.
    */
    void EcLang::constructFromBinary(std::vector<uint8_t> compiled) {
        // Remove language bytes
        // We remove the (number of identifier bytes + 0 (null termination)) first bytes
        compiled.erase(compiled.begin(), compiled.begin()+language->getIdentifierBytes().size()+1);

        // Current Class ID
        uint8_t classID = 0;

        for (size_t i = 0; i < compiled.size(); i++) {
            // 7 types of instructions
            switch (compiled.at(i)) {
            case INST_CREATE: {
                // Get Class Name
                i++;
                classID = compiled.at(i);
                std::string className = language->getClassName(classID);
                // Get Object Name
                i++;
                std::string objectName = decompileString(compiled, i).substr(1); // Remove prefix
                // Create
                objects.push_back(new Object(className, objectName, currentFile));
                break;
            }
            case INST_ATTRIBUTE: {
                i++;
                Attribute* attribute = createAttributeFromBinary(compiled, i, classID, language);
                break;
            }
            case INST_SCOPE_ENTER: {
                // Get last object saved and put it in scope
                if (objects.empty()) { break; }
                scope.push_back(objects.at(objects.size()-1));
                break;
            }
            case INST_SCOPE_EXIT: {
                if (scope.empty()) { break; }
                scope.pop_back();
                break;
            }
            case INST_INCLUDE: {
                // Get File we're including
                i++;
                std::string file = decompileString(compiled, i).substr(1);

                uint8_t includedFile = (currentFile==0) ? includedFilenames.size() : currentFile;

                #ifdef ECLANG_DEBUG 
                if (currentFile != 0) {
                    std::cout << "ECLANG_LOG: Dynamic inclusion detected in dynamically included file... including statically: "+file+"\n";
                } else {
                    std::cout << "ECLANG_LOG: Dynamically including file: "+file+"\n";
                }
                #endif

                EcLang includedEcLang(file, includedFile);
                
                // We only have to do this if this is the file included by the user.
                // We register the path so that the file ID actually points to something
                // The extra "i" indicates that this is an Include, not a Template
                if (currentFile == 0) {
                    includedFilenames.push_back("i"+file);
                }

                // Include into our current scene
                std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                // Add to current object in scope OR simply add to root
                if (scope.empty()) {
                    objects.insert(objects.end(), children.begin(), children.end());
                } else {
                    scope.at(scope.size()-1)->_addChildren(children);
                }
                break;
            }
            case INST_TEMPLATE: {
                // We can't use multiple templates in the same file
                if (!externalTemplateNode.empty()) {
                    throw std::runtime_error("ECLANG_ERROR: Using multiple templates (statically or dynamically) in a single file is not permitted.\n");
                }

                // Get File we're including
                i++;
                std::string file = decompileString(compiled, i).substr(1);
                
                uint8_t includedFile = (currentFile==0) ? includedFilenames.size() : currentFile;

                #ifdef ECLANG_DEBUG 
                if (currentFile != 0) {
                    std::cout << "ECLANG_LOG: Dynamic inclusion detected in dynamically included Template file... including statically: "+file+"\n";
                } else {
                    std::cout << "ECLANG_LOG: Dynamically including Template file: "+file+"\n";
                }
                #endif

                EcLang includedEcLang(file, includedFile);
                
                // Register filename
                if (currentFile == 0) {
                    includedFilenames.push_back("t"+file);
                }

                // Include into our current scene
                std::vector<Object*> children = includedEcLang._getAllObjectsAsInclude();
                externalTemplateNode = includedEcLang._getTemplateNodePath();
                // Add to current object in scope OR simply add to root
                if (scope.empty()) {
                    objects.insert(objects.end(), children.begin(), children.end());
                } else {
                    scope.at(scope.size()-1)->_addChildren(children);
                }
                // Add template node to current scope (even if any of the vectors is empty)
                scope.insert(scope.end(), externalTemplateNode.begin(), externalTemplateNode.end());
                break;
            }
            case INST_MARK_TEMPLATE: {
                templateNode = scope;
                break;
            }
            }
        }
    }

    /**
        returns a vector of uint8_t containing the compiled file.

        This method is called when calling saveToFileCompiled() if the file is a source file.

        WHAT IT DOES
        1. Save Identifier Bytes for the language and NULL-terminate

        2. For each object in the array, check what file it belongs to. Is the File ID 0?
            T: Then this object is part of this file or was statically included
                2.T.1. Save the object name (NULL-terminated) after the [CREATE] instruction (see below)
                    2.T.1.A. To save time, I skipped the checks about adding [SCOPE_ENTER] and [SCOPE_EXIT] instructions.
                        These instructions are added if the object has children, has attributes or is a Template Node.
                        If the [SCOPE_ENTER] has been used, we should add a [SCOPE_EXIT] at the end

                2.T.2. Does this object have children?
                    T: We should run this entire section for each of the children again (recursively)
                        2.T.2.T.1. Get all children of the object and repeat point [2] for each child

                2.T.3. Register all attributes with the [ATTRIBUTE] instruction followed by the Attribute ID and
                    the value (ID and size of value depends on the language)

                2.T.4. Is this object the same as the last one in `templateNode` (if it's not empty)?
                    T: Then we should mark it 
                        2.T.4.T.1. Add a [MARK_TEMPLATE] instruction

            
            F: This file was included Dynamically. We know if the file is an include or a template because of the first character of the file name
                2.F.1. Add a [TEMPLATE] or [INCLUDE] instruction with the file name (NULL-terminated)
                    2.F.1.A. If the file is a Template, we register it so that we can ignore any other node also coming from that file later on
                    2.F.1.B. If the file is a Template and we already included a Template, the compilation fails.

                2.F.2. DO NOT ADD ANY OBJECT TO THE BINARY!
                2.F.3. Continue consuming objects until we see that the next object has a different file ID.
                2.F.4. When we find a different file ID we stop consuming. Continue.


        INSTRUCTIONS
        - INST_CREATE:        [0x00] - Takes the Language-specific Class ID (or 255 and then a STRING with the Class Name) and the name as a STRING as parameter.
        - INST_ATTRIBUTE:     [0x01] - Language-dependent; Takes Attribute ID (UINT8_T) and a Language-specific value
        - INST_SCOPE_ENTER:   [0x02] - Takes nothing. Enters the scope of the last created Object
        - INST_SCOPE_EXIT:    [0x03] - Takes nothing. Returns to the parent node's scope
        - INST_INCLUDE:       [0x04] - Takes STRING with the file name or alias. Represents a Dynamic Include (`include-dyn`)
        - INST_TEMPLATE:      [0x05] - Takes STRING with the file name or alias. Represents a Dynamic Template (`template-dyn`)
        - INST_MARK_TEMPLATE: [0x06] - Takes nothing. Marks the current node as Template Node
        
        DATA TYPES
        The only strange one are Strings because of their variable size:
        - STRING: Starts with a [0x00] to indicate that it's a normal String, then it continues until the next 0.
        - STRING_MD: Starts with a [0x01] to indicate that it's a Markdown String, then it continues until the next 0.
    */
    std::vector<uint8_t> EcLang::compile() {
        std::vector<uint8_t> binary;

        // Insert identifier bytes
        std::vector<uint8_t> languageIdentifier = language->getIdentifierBytes();
        binary.insert(binary.end(), languageIdentifier.begin(), languageIdentifier.end());
        binary.push_back(uint8_t(0));

        // Save all objects (this function takes care of includes and templates)
        auto compiledObjects = compileObjects(objects);
        binary.insert(binary.end(), compiledObjects.begin(), compiledObjects.end());

        return binary;
    }
    /**
        returns a string containing the decompiled source code.

        This method is called when calling saveToFileSource().
        If this method fails to execute, an exception will be thrown.
        Errors can happen if the Language used for compilation is not the same as the
        one used for decompilation (for example due to updates to the language)
    */
    std::string EcLang::decompile() {
        std::string source;

        // Set language
        source += "#language " + language->getName() + "\n\n";
        // Get all objects
        source += decompileObjects(objects);

        return source;
    }

    // COMPILATION / DECOMPILATION HELPER FUNCTIONS

    /**
        Takes a pointer to an instance of Object and returns its compiled code.
        This function is recursive and will produce the compiled code for setting
        the attributes, setting the template tag and creating children objects
    */
    std::vector<uint8_t> EcLang::compileObjects(std::vector<Object*> objects) {
        // Create container for binary code
        std::vector<uint8_t> binary;

        for (size_t i = 0; i < objects.size(); i++) {
            Object* object = objects.at(i);

            if (object->getSourceFileID() == 0) {
                // Insert Object Creation Instruction
                std::vector<uint8_t> compiledObjectCreation = compileObjectCreation(object);
                binary.insert(binary.end(), compiledObjectCreation.begin(), compiledObjectCreation.end());

                // Check for children, attributes or template tag
                auto children = object->getObjects();
                auto attributes = object->getAttributes();
                // We check if the last element in template node is the same as the current node, if not, this is not a template node
                bool isTemplate = templateNode.empty() ? false : templateNode.at(templateNode.size()-1) == object;

                // should we have a SCOPE_ENTER? Yes but only if the object has children, attributes or is a template node
                bool shouldEnterScope = false;
                if (!children.empty() || !attributes.empty() || isTemplate) {
                    shouldEnterScope = true;
                    // Also enter scope, we will use shouldEnterScope to see if we should close it at the end
                    binary.push_back(INST_SCOPE_ENTER);
                }

                // Set as Template Node
                if (isTemplate) {
                    binary.push_back(INST_MARK_TEMPLATE);
                }

                // Register attributes
                if (!attributes.empty()) {
                    auto attributes = object->getAttributes();
                    for (std::string attribute : attributes) {
                        // Getting the attribute might fail if the attribute is not registered for that class
                        // or if the class is not registered in that language. In any of those cases we save
                        // the attribute as a custom attribute (save INST_ATTR_CUSTOM and a STRING)
                        try {
                            uint8_t attributeID = language->getAttributeID(object->getClassName(), attribute);
                            // Save instruction and attribute ID
                            binary.push_back(INST_ATTRIBUTE);
                            binary.push_back(attributeID);
                            // Save value (shit)
                            // TODO: Do something with this shit (figure out a cleaner way that doesn't involve filling a switch with 23 elements by hand)
                            uint8_t objectAttributeID = object->getIDOf(attribute);
                            switch (language->getAttributeType(object->getClassName(), attribute)) {
                            case type::INT8: {
                                auto vector = numberToBytes(object->getInt8Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::INT16: {
                                auto vector = numberToBytes(object->getInt16Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::INT32: {
                                auto vector = numberToBytes(object->getInt32Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::INT64: {
                                auto vector = numberToBytes(object->getInt64Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::UINT8: {
                                auto vector = numberToBytes(object->getUint8Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::UINT16: {
                                auto vector = numberToBytes(object->getUint16Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::UINT32: {
                                auto vector = numberToBytes(object->getUint32Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::UINT64: {
                                auto vector = numberToBytes(object->getUint64Of(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::FLOAT: {
                                auto vector = numberToBytes(object->getFloatOf(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::DOUBLE: {
                                auto vector = numberToBytes(object->getDoubleOf(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::STRING: {
                                auto vector = compileString(object->getStringOf(objectAttributeID));
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::STR_MD: {
                                auto vector = compileString(object->getStringOf(objectAttributeID), true);
                                binary.insert(binary.end(), vector.begin(), vector.end());
                            }   break;
                            case type::VEC2I: {
                                vec2i v = object->getVec2iOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                            }   break;
                            case type::VEC3I: {
                                vec3i v = object->getVec3iOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                            }   break;
                            case type::VEC4I: {
                                vec4i v = object->getVec4iOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                auto num4 = numberToBytes(v.w);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                                binary.insert(binary.end(), num4.begin(), num4.end());
                            }   break;
                            case type::VEC2L: {
                                vec2l v = object->getVec2lOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                            }   break;
                            case type::VEC3L: {
                                vec3l v = object->getVec3lOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                            }   break;
                            case type::VEC4L: {
                                vec4l v = object->getVec4lOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                auto num4 = numberToBytes(v.w);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                                binary.insert(binary.end(), num4.begin(), num4.end());
                            }   break;
                            case type::VEC2F: {
                                vec2f v = object->getVec2fOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                            }   break;
                            case type::VEC3F: {
                                vec3f v = object->getVec3fOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                            }   break;
                            case type::VEC4F: {
                                vec4f v = object->getVec4fOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                auto num4 = numberToBytes(v.w);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                                binary.insert(binary.end(), num4.begin(), num4.end());
                            }   break;
                            case type::VEC2D: {
                                vec2d v = object->getVec2dOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                            }   break;
                            case type::VEC3D: {
                                vec3d v = object->getVec3dOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                            }   break;
                            case type::VEC4D: {
                                vec4d v = object->getVec4dOf(objectAttributeID);
                                auto num1 = numberToBytes(v.x);
                                auto num2 = numberToBytes(v.y);
                                auto num3 = numberToBytes(v.z);
                                auto num4 = numberToBytes(v.w);
                                binary.insert(binary.end(), num1.begin(), num1.end());
                                binary.insert(binary.end(), num2.begin(), num2.end());
                                binary.insert(binary.end(), num3.begin(), num3.end());
                                binary.insert(binary.end(), num4.begin(), num4.end());
                            }   break;
                            }
                        } catch (...) {
                            // Uh oh! This instruction wasn't registered for this class (or the class wasn't registered) (probably)
                            // This part maaaayyyy be called if something above fails but it's probably not a problem (it would indeed be a problem)
                            // TODO: Fix this entire function, it's hacky and ugly as hell (more information in comment above and other TODOs)
                            // Save instruction and attribute ID
                            binary.push_back(INST_ATTRIBUTE);
                            binary.push_back(INST_ATTR_CUSTOM);
                            auto attributeName = compileString(attribute);
                            binary.insert(binary.end(), attributeName.begin(), attributeName.end());
                            // Calling getStringOf() on an attribute that is not a String or Markdown String returns whatever the value is (even vectors) as a String
                            auto valueString = compileString(object->getStringOf(object->getIDOf(attribute)));
                            binary.insert(binary.end(), valueString.begin(), valueString.end());
                        }
                    }
                } // register attributes

                // If this object has children
                if (!children.empty()) {
                    // We need to pass the children directly to this function (recursively).
                    // This already takes care of things like includes and templates.
                    auto compiledChildren = compileObjects(children);
                    binary.insert(binary.end(), compiledChildren.begin(), compiledChildren.end());
                }

                // Close scope
                if (shouldEnterScope) {
                    binary.push_back(INST_SCOPE_EXIT);
                }

            } else {
                // Templates exist and require extra code. If the file is a template we need to get its Template Node
                // and run `compileObject()` for every child Node with a File ID of 0
                const uint8_t fileID = object->getSourceFileID();
                const std::string filenameWithType = includedFilenames.at(fileID);
                const std::string filename = filenameWithType.substr(1);

                // 't' for template, 'i' for include
                if (filenameWithType.at(0) == 't') {
                    // Create the instruction
                    binary.push_back(INST_TEMPLATE);
                    auto compiledFilename = compileString(filename);
                    binary.insert(binary.end(), compiledFilename.begin(), compiledFilename.end());

                    // we get the Template Node and run this function on its children
                    // INFO: There can be only one Template included so it's OK (albeit a bit hacky) to just read the Temaplate Node object saved into this object
                    // We then IGNORE all the remaining nodes
                    // INFO: We are IGNORING all nodes that come after the Template node.
                    // This is fine because anything below a Template inclusion goes into its Template Node
                    auto compiledChildren = compileObjects(externalTemplateNode.at(externalTemplateNode.size()-1)->getObjects());
                    binary.insert(binary.end(), compiledChildren.begin(), compiledChildren.end());
                    // We break from the entire loop in order to ignore everything after #template
                    break;
                }
                else {
                    // First, create the instruction
                    binary.push_back(INST_INCLUDE);
                    auto compiledFilename = compileString(filename);
                    binary.insert(binary.end(), compiledFilename.begin(), compiledFilename.end());

                    // Now we just need to continue until we find an object with a different File ID
                    // We start from this object and continue until we see a different File ID or the vector exits normally
                    // We then set `i` to be the same as `objectEvaluating` so that we can continue the function normally
                    for (size_t objectEvaluating = i; objectEvaluating < objects.size(); objectEvaluating++) {
                        Object* eval = objects.at(objectEvaluating);
                        if (eval->getSourceFileID() == fileID) {
                            continue;
                        }
                        // The file ID changed! We don't know if we included another file or we're at File ID 0
                        // but we don't care, we'll take care of that in the next iteration of the main loop.
                        i = objectEvaluating-1;
                        break;
                    }
                    // The next iteration of this main loop will be AFTER all the included Nodes
                }
            }
        }

        return binary;
    }
    /**
        Takes a pointer to an instance of Object and returns the CREATE instruction
        for it.
    */
    std::vector<uint8_t> EcLang::compileObjectCreation(Object* object) {
        std::vector<uint8_t> binary;

        // Add object creation instruction for this object
        binary.push_back(INST_CREATE);

        // Add Class Name to Binary
        if (language->classExists(object->getClassName())) {
            binary.push_back(language->getClassID(object->getClassName()));
        } else {
            // It is possible to use a made up class name instead of a language-specific one
            // This is a special case. We insert a INST_ATTR_CUSTOM into the binary, which indicates that we're using
            // a custom class, then we insert a STRING with the class name
            // TODO: Parser should give a warning when unregistered classes are used
            binary.push_back(INST_ATTR_CUSTOM);
            // Insert STRING with Class Name
            std::vector<uint8_t> className = compileString(object->getClassName());
            binary.insert(binary.end(), className.begin(), className.end());
        }

        // Add Object Name to Binary
        std::vector<uint8_t> objectName = compileString(object->getName());
        binary.insert(binary.end(), objectName.begin(), objectName.end());

        return binary;
    }

    /**
        Takes a pointer to an instance of Object and returns its decompiled code.
        This function is recursive and will produce the decompiled code for setting
        the attributes, setting the template tag and creating children objects
    */
    std::string EcLang::decompileObjects(std::vector<Object*> objects, uint8_t tabs) {
        std::string decompiled;
        // Tabulation. 1 tab = 4 spaces
        std::string spacing = "";
        for (uint8_t i = 0; i < tabs; i++) {
            spacing += "    ";
        }

        for (size_t i = 0; i < objects.size(); i++) {
            Object* object = objects.at(i);

            if (object->getSourceFileID() == 0) {
                // Insert Object Creation Instruction
                decompiled += spacing + object->getClassName() + " " + object->getName();

                // Check for children, attributes or template tag
                auto children = object->getObjects();
                auto attributes = object->getAttributes();
                // We check if the last element in template node is the same as the current node, if not, this is not a template node
                bool isTemplate = templateNode.empty() ? false : templateNode.at(templateNode.size()-1) == object;

                // should we have a SCOPE_ENTER? Yes but only if the object has children, attributes or is a template node
                bool shouldEnterScope = false;
                if (!children.empty() || !attributes.empty() || isTemplate) {
                    shouldEnterScope = true;
                    // Also enter scope, we will use shouldEnterScope to see if we should close it at the end
                    decompiled += " {\n";
                }

                // Set as Template Node
                if (isTemplate) {
                    decompiled += spacing + "    #template\n";
                }

                // Register attributes
                if (!attributes.empty()) {
                    auto attributes = object->getAttributes();
                    for (std::string attribute : attributes) {
                        std::string value;
                        if (language->getAttributeType(object->getClassName(), attribute) == type::STRING) {
                            value = "\""+object->getStringOf(object->getIDOf(attribute))+"\"";
                        } else if (language->getAttributeType(object->getClassName(), attribute) == type::STR_MD) {
                            value = "`"+object->getStringOf(object->getIDOf(attribute))+"`";
                        } else {
                            value = object->getStringOf(object->getIDOf(attribute));
                        }
                        decompiled += spacing + "    " + attribute + " = " + value + ";\n";
                    }
                } // register attributes

                // If this object has children
                if (!children.empty()) {
                    // We need to pass the children directly to this function (recursively).
                    // This already takes care of things like includes and templates.
                    std::string decompiledChildren = decompileObjects(children, tabs+1);
                    decompiled += decompiledChildren;
                }

                // Close scope
                if (shouldEnterScope) {
                    decompiled += spacing + "}\n";
                } else {
                    decompiled += ";\n";
                }

            } else {
                // Templates exist and require extra code. If the file is a template we need to get its Template Node
                // and run `compileObject()` for every child Node with a File ID of 0
                const uint8_t fileID = object->getSourceFileID();
                const std::string filenameWithType = includedFilenames.at(fileID);
                const std::string filename = filenameWithType.substr(1);

                // 't' for template, 'i' for include
                if (filenameWithType.at(0) == 't') {
                    decompiled += spacing + "#template-dyn \"" + filename + "\"\n";

                    // INFO: See compileObjects() for more information about why we do this
                    std::string decompiledChildren = decompileObjects(externalTemplateNode.at(externalTemplateNode.size()-1)->getObjects(), tabs);
                    decompiled += decompiledChildren;
                    // We break from the entire loop in order to ignore everything after #template
                    break;
                }
                else {
                    // First, create the instruction
                    decompiled += spacing + "#include-dyn \"" + filename + "\"\n";

                    // Now we just need to continue until we find an object with a different File ID
                    // We start from this object and continue until we see a different File ID or the vector exits normally
                    // We then set `i` to be the same as `objectEvaluating` so that we can continue the function normally
                    for (size_t objectEvaluating = i; objectEvaluating < objects.size(); objectEvaluating++) {
                        Object* eval = objects.at(objectEvaluating);
                        if (eval->getSourceFileID() == fileID) {
                            continue;
                        }
                        // The file ID changed! We don't know if we included another file or we're at File ID 0
                        // but we don't care, we'll take care of that in the next iteration of the main loop.
                        i = objectEvaluating-1;
                        break;
                    }
                    // The next iteration of this main loop will be AFTER all the included Nodes
                }
            }
        }

        return decompiled;
    }

    /**
        Converts a number to an array of bytes
    */
    std::vector<uint8_t> EcLang::numberToBytes(int8_t number) {
        return {*reinterpret_cast<uint8_t*>(&number)};
    }
    std::vector<uint8_t> EcLang::numberToBytes(int16_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(int32_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(int64_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(uint8_t number) {
        return {*reinterpret_cast<uint8_t*>(&number)};
    }
    std::vector<uint8_t> EcLang::numberToBytes(uint16_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(uint32_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(uint64_t number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(float number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3]};
    }
    std::vector<uint8_t> EcLang::numberToBytes(double number) {
        uint8_t* data = reinterpret_cast<uint8_t*>(&number);
        return {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]};
    }
}