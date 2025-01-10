#include "classes/language.hpp"

// std
#include <algorithm>
#include <stdexcept>
#include <string>

namespace eclang {
    /**
        Language constructor with the name of the language and the file extensions.
        It also contains the first bytes of the file. They are used to identify the language
        when reading a compiled file. If any of the inputted bytes is 0 an exception is thrown
    */
    Language::Language(std::string name, std::string sourceFileExtension, std::string compiledFileExtension, std::vector<uint8_t> identifierBytes) {
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

    // Get language data
    // -----------------
    /**
        Returns the name of the language. The name is used to select
        the language when creating an EcLang object
    */
    std::string Language::getName() {
        return name;
    }
    /**
        Returns the file extension of source files (uncompiled code) for this language.
    */
    std::string Language::getExtensionSource() {
        return sourceFileExtension;
    }
    /**
        Returns the file extension of compiled files for this language
    */
    std::string Language::getExtensionCompiled() {
        return compiledFileExtension;
    }
    /**
        Returns the first bytes of compiled files that use this language.
        They are used to identify the language of a compiled EcLang file.
    */
    std::vector<uint8_t> Language::getIdentifierBytes() {
        return identifierBytes;
    }

    // Register Classes
    // ----------------
    /**
        Registers a class into the language.
    */
    void Language::registerClass(Class c) {
        classes.push_back(c);
        classesStr.push_back(c.getName());
    }

    // Classes
    // -------
    /**
        Returns a list of all registered classes in the language
    */
    std::vector<std::string> Language::getClasses() {
        return classesStr;
    }
    /**
        Returns true if a class with the ID specified exists
    */
    bool Language::classExists(uint32_t id) {
        return id < classes.size();
    }
    /**
        Returns true if a class with the name specified exists
    */
    bool Language::classExists(std::string name) {
        return std::find(classesStr.begin(), classesStr.end(), name) != classesStr.end();
    }
    /**
        Returns the class name from its ID
    */
    std::string Language::getClassName(uint32_t id) {
        if (!classExists(id)) {
            throw std::runtime_error("ECLANG_ERROR: Tried to get name of a class with invalid ID ("+std::to_string(id)+").");
        }
        return classesStr.at(id);
    }
    /**
        Returns the ID of the class from its name
    */
    uint32_t Language::getClassID(std::string name) {
        auto iterator = std::find(classesStr.begin(), classesStr.end(), name);
        if (iterator == classesStr.end()) {
            throw std::runtime_error("ECLANG_ERROR: Tried to get the ID of a class with unregistered name ("+name+").");
        }
        return iterator - classesStr.begin();
    }

    // Attributes from classes
    // -----------------------
    /**
        Takes the ID of the class and the ID of an attribute
        and returns the name of the attribute
    */
    std::string Language::getAttributeName(uint32_t classID, uint32_t attributeID) {
        return classes.at(classID).getAttributeName(attributeID);
    }
    /**
        Takes the name of the class and the ID of an attribute
        and returns the name of the attribute
    */
    std::string Language::getAttributeName(std::string className, uint32_t attributeID) {
        return classes.at(getClassID(className)).getAttributeName(attributeID);
    }

    /**
        Takes the ID of the class and the name of an attribute
        and returns the ID of the attribute
    */
    uint32_t Language::getAttributeID(uint32_t classID, std::string attributeName) {
        return classes.at(classID).getAttributeID(attributeName);
    }
    /**
        Takes the name of the class and the name of an attribute
        and returns the ID of the attribute
    */
    uint32_t Language::getAttributeID(std::string className, std::string attributeName) {
        return classes.at(getClassID(className)).getAttributeID(attributeName);
    }

    /**
        Takes the ID of the class and the ID of an attribute
        and returns the data type of the attribute
    */
    type::Type Language::getAttributeType(uint32_t classID, uint32_t attributeID) {
        return classes.at(classID).getAttributeType(attributeID);
    }
    /**
        Takes the ID of the class and the name of an attribute
        and returns the data type of the attribute
    */
    type::Type Language::getAttributeType(uint32_t classID, std::string attributeName) {
        return classes.at(classID).getAttributeType(attributeName);
    }
    /**
        Takes the name of the class and the ID of an attribute
        and returns the data type of the attribute
    */
    type::Type Language::getAttributeType(std::string className, uint32_t attributeID) {
        return classes.at(getClassID(className)).getAttributeType(attributeID);
    }
    /**
        Takes the name of the class and the name of an attribute
        and returns the data type of the attribute
    */
    type::Type Language::getAttributeType(std::string className, std::string attributeName) {
        return classes.at(getClassID(className)).getAttributeType(attributeName);
    }
}