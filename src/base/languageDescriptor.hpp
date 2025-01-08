#pragma once

// eclang
#include "class.hpp"

// std
#include <cstdint>
#include <string>
#include <vector>

namespace eclang::language {
    /**
        Information that allows an EcLang object to understand
        how a custom EcLang file (compiled or not) should be interpreted
        and/or compiled.

        Manual creation of Language Descriptors is only necessary for
        files that are not AUII or NEA as these types have predefined
        Language Descriptors
    */
    class LanguageDescriptor {
    public:
        /**
            Language Descriptor Constructor with the name of the language and the file extensions.
            It also contains the first bytes of the file. They are used to identify the language
            when reading a compiled file. If any of the inputted bytes is 0 an exception is thrown
        */
        LanguageDescriptor(std::string name, std::string sourceFileExtension, std::string compiledFileExtension, std::vector<uint8_t> identifierBytes);

    private:
        // Language name
        std::string name;

        // File extensions are NOT used to identify the language of a file.
        // They're used when saving compiled/decompiled files
        std::string sourceFileExtension;
        std::string compiledFileExtension;
        
        // First bytes of the compiled file. It acts as an identifier for the language
        // A byte in this vector can never be 0, as this sequence is NULL terminated.
        std::vector<uint8_t> identifierBytes;

        // A list of possible Node Types (called classes).
        // These classes can have zero or more parameters that can be of one of several
        // data types.
        std::vector<Class> classes;
    };
}