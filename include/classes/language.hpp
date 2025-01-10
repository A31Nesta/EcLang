#pragma once

// eclang
#include "class.hpp"

// std
#include <cstdint>
#include <string>
#include <vector>

namespace eclang {
    /**
        Information that allows an EcLang object to understand
        how a custom EcLang file (compiled or not) should be interpreted
        and/or compiled.

        Manual creation of Language objects is only necessary for
        files that are not AUII or NEA as these types have predefined
        Language objects
    */
    class Language {
    public:
        /**
            Language constructor with the name of the language and the file extensions.
            It also contains the first bytes of the file. They are used to identify the language
            when reading a compiled file. If any of the inputted bytes is 0 an exception is thrown
        */
        Language(std::string name, std::string sourceFileExtension, std::string compiledFileExtension, std::vector<uint8_t> identifierBytes);
        /**
            Language constructor with the name of the language and the file extensions.
            It also contains the first bytes of the file. They are used to identify the language
            when reading a compiled file. If any of the inputted bytes is 0 an exception is thrown.

            This version allows you to put classes directly, creating the entire language object in one single line
        */
        Language(std::string name, std::string sourceFileExtension, std::string compiledFileExtension, std::vector<uint8_t> identifierBytes, std::vector<Class> classes);

        // Get language data
        // -----------------
        /**
            Returns the name of the language. The name is used to select
            the language when creating an EcLang object
        */
        std::string getName();
        /**
            Returns the file extension of source files (uncompiled code) for this language.
        */
        std::string getExtensionSource();
        /**
            Returns the file extension of compiled files for this language
        */
        std::string getExtensionCompiled();
        /**
            Returns the first bytes of compiled files that use this language.
            They are used to identify the language of a compiled EcLang file.
        */
        std::vector<uint8_t> getIdentifierBytes();

        // Register Classes
        // ----------------
        /**
            Registers a class into the language.
        */
        void registerClass(Class c);

        // Classes
        // -------
        /**
            Returns a list of all registered classes in the language
        */
        std::vector<std::string> getClasses();
        /**
            Returns true if a class with the ID specified exists
        */
        bool classExists(uint32_t id);
        /**
            Returns true if a class with the name specified exists
        */
        bool classExists(std::string name);
        /**
            Returns the class name from its ID
        */
        std::string getClassName(uint32_t id);
        /**
            Returns the ID of the class from its name
        */
        uint32_t getClassID(std::string name);

        // Attributes from classes
        // -----------------------
        /**
            Takes the ID of the class and the ID of an attribute
            and returns the name of the attribute
        */
        std::string getAttributeName(uint32_t classID, uint32_t attributeID);
        /**
            Takes the name of the class and the ID of an attribute
            and returns the name of the attribute
        */
        std::string getAttributeName(std::string className, uint32_t attributeID);

        /**
            Takes the ID of the class and the name of an attribute
            and returns the ID of the attribute
        */
        uint32_t getAttributeID(uint32_t classID, std::string attributeName);
        /**
            Takes the name of the class and the name of an attribute
            and returns the ID of the attribute
        */
        uint32_t getAttributeID(std::string className, std::string attributeName);

        /**
            Takes the ID of the class and the ID of an attribute
            and returns the data type of the attribute
        */
        type::Type getAttributeType(uint32_t classID, uint32_t attributeID);
        /**
            Takes the ID of the class and the name of an attribute
            and returns the data type of the attribute
        */
        type::Type getAttributeType(uint32_t classID, std::string attributeName);
        /**
            Takes the name of the class and the ID of an attribute
            and returns the data type of the attribute
        */
        type::Type getAttributeType(std::string className, uint32_t attributeID);
        /**
            Takes the name of the class and the name of an attribute
            and returns the data type of the attribute
        */
        type::Type getAttributeType(std::string className, std::string attributeName);

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
        // These classes can have zero or more attributes that can be of one of several
        // data types.
        std::vector<Class> classes;
        std::vector<std::string> classesStr;
    };
}