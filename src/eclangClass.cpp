#include "classes/eclangClass.hpp"
// eclang
#include "classes/language.hpp"
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
            case lexer::type::ASSIGNMENT:
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
    std::vector<Object> EcLang::getAllObjects() {
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
                }
            }break;
            case lexer::type::CLASS: {
                // CLASS should be followed by IDENTIFIER and optionally SCOPE_ENTER.
                // If SCOPE_ENTER is ommited we should have a SEMICOLON.
                // Either way, we need 2 arguments
                const lexer::Token& identifier = tokens.at(current+1);
                const lexer::Token& terminator = tokens.at(current+2); // again, semicolon or enter scope
                if (identifier.type != lexer::type::IDENTIFIER) {
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+identifier.string+"\" at column "+std::to_string(identifier.column)+" at line "+std::to_string(identifier.line)+". Usage: <Class> <Identifier>; or <Class> <Identifier> {}\n";
                    break;
                }
                if (terminator.type == lexer::type::SEMICOLON) {
                    // TODO: Create object and NOT make this node the new scope.
                }
                else if (terminator.type == lexer::type::SCOPE_ENTER) {
                    // TODO: Create object and make this node the new scope.
                }
                else {
                    std::cerr << "ECLANG_ERROR: Unexpected token \""+terminator.string+"\" at column "+std::to_string(terminator.column)+" at line "+std::to_string(terminator.line)+". Semicolon or curly braces were expected after Node declaration.\n";
                }
            }break;
            case lexer::type::IDENTIFIER:
                // This is where things get complex
                parseIdentifier();
            default:
                // Everything that does not start an instruction goes through here.
                // If something is not starting an instruction then it shouldn't be here.
                hasErrors = true;
                std::cerr << "ECLANG_ERROR: Unexpected token \""+t.string+"\" at column "+std::to_string(t.column)+" at line "+std::to_string(t.line)+".\n";
                break;
            }
        }
    }
    /**
        Constructs all the Object objects by reading a binary file.
    */
    void EcLang::constructFromBinary(std::vector<uint8_t> compiled) {
        // TODO: Implement reading binary file
    }

    void EcLang::parseIdentifier() {
        // TODO: Implement this monstruous function
    }
}