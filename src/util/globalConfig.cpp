#include "globalConfig.hpp"
#include "classes/attribute.hpp"
#include <vector>

namespace eclang::config {
    // OBJECTS
    // -------
    std::vector<Language> languages {
        // EcLanguage Descriptor
        // ---------------------
        {
            "eclang-descriptor", "eld", "eldc", {'E', 'C', 'L', 'D', 0x31}, {
                // CLASSES
                {"Language", {
                    // ATTRIBUTES
                    // {"name", type::STRING}, // Name is not an attribute, you set the name with `Language myLanguage {}` from the ELD file
                    {"sourceExtension", type::STRING},
                    {"binaryExtension", type::STRING},
                    {"identifierBytes", type::STRING} // Comma separated values, can be chars ('X') or numbers (dec or hex). Numbers can't be greater than 255 or less than 1.
                }},
                {"Class"}, // Class doesn't have attributes, it has a collection of Attributes (the class)
                {"Attribute", {
                    {"type", type::STRING}
                }}
            }
        }
        // AUII
        // ----
        // {
        //     "AUII", "auii", "auic", {'A', 'U', 'I', 'I', 0x31}
        // }
    };

    // "PRIVATE" FUNCTIONS
    // -------------------

    // HEADER DEFINITIONS
    // ------------------

    /**
        Returns all registered languages.
    */
    std::vector<Language> getLanguages() {
        return languages;
    }
    /**
        Registers a language.
        Allows you to use this language for
        compilation/decompilation/interpretation
    */
    void registerLanguage(Language lang) {
        languages.push_back(lang);
    }
}