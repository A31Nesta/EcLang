#include "languageDescriptor.hpp"

// std
#include <algorithm>
#include <stdexcept>

namespace eclang::language {
    /**
        Language Descriptor Constructor with the name of the language and the file extensions.
        It also contains the first bytes of the file. They are used to identify the language
        when reading a compiled file. If any of the inputted bytes is 0 an exception is thrown
    */
    LanguageDescriptor::LanguageDescriptor(std::string name, std::string sourceFileExtension, std::string compiledFileExtension, std::vector<uint8_t> identifierBytes) {
        // Save Strings
        this->name = name;
        this->sourceFileExtension = sourceFileExtension;
        this->compiledFileExtension = compiledFileExtension;

        // If a 0 is found in the identifierBytes vector, throw error
        if (std::find(identifierBytes.begin(), identifierBytes.end(), uint8_t(0)) != identifierBytes.end()) {
            throw std::runtime_error("ECLANG_ERROR: 0x0 specified as part of identifier bytes in EcLangDescriptor");
        }

        // Save bytes
        this->identifierBytes = identifierBytes;
    }
}