#include "globalConfig.hpp"
#include "classes/attribute.hpp"
#include <stdexcept>
#include <unordered_map>
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
        },
        // Debug EcLang
        // ------------
        {
            "test", "elt", "eltc", {'E', 'C', 'L', 'T', 0x00000031}, {
                {"Container"}, // Empty, just to debug putting nodes inside nodes
                // DEBUG ASSIGN
                {"StringTests", { // Debug Strings
                    {"string", type::STRING},
                    {"stringMD", type::STR_MD},
                }},
                {"NumberTests", { // Debug Numbers
                    {"int8", type::INT8},
                    {"int16", type::INT16},
                    {"int32", type::INT32},
                    {"int64", type::INT64},

                    {"uint8", type::UINT8},
                    {"uint16", type::UINT16},
                    {"uint32", type::UINT32},
                    {"uint64", type::UINT64},

                    {"float", type::FLOAT},
                    {"double", type::DOUBLE}
                }},
                {"VectorTests", { // Debug Vectors
                    {"vec2f", type::VEC2F},
                    {"vec3f", type::VEC3F},
                    {"vec4f", type::VEC4F},

                    {"vec2d", type::VEC2D},
                    {"vec3d", type::VEC3D},
                    {"vec4d", type::VEC4D},

                    {"vec2i", type::VEC2I},
                    {"vec3i", type::VEC3I},
                    {"vec4i", type::VEC4I},

                    {"vec2l", type::VEC2L},
                    {"vec3l", type::VEC3L},
                    {"vec4l", type::VEC4L}
                }}
            }
        }
    };

    std::unordered_map<std::string, std::string> aliasToPath;

    // "PRIVATE" FUNCTIONS
    // -------------------

    // HEADER DEFINITIONS
    // ------------------

    /**
        Returns all registered languages.
    */
    std::vector<Language>& getLanguages() {
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

    /**
        Adds an alias to a file path.
        Whether or not the path is valid is not checked.
    */
    void filepathRegister(std::string alias, std::string path) {
        aliasToPath[alias] = path;
    }
    /**
        Is this string an alias?
    */
    bool filepathIsAlias(std::string alias) {
        // Since C++ 20 we could use .contains()
        return aliasToPath.find(alias) != aliasToPath.end();
    }
    /**
        Returns the file path for the given alias
    */
    std::string filepathGetFor(std::string alias) {
        if (!filepathIsAlias(alias)) throw std::runtime_error("ECLANG_ERROR: Tried to obtain filepath for invalid alias.");
        return aliasToPath[alias];
    }
}