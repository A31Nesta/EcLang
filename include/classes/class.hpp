#pragma once

// eclang
#include "attribute.hpp"

// std
#include <cstdint>
#include <string>
#include <vector>
namespace eclang {
    /**
        A class in EcLang.
        Classes in EcLang are structures with a specific name
        that can contain attributes.

        This class is used for creating Languages
    */
    class Class {
    public:
        /**
            Constructs the object with the name of the class
        */
        Class(std::string name);
        /**
            Constructs the object with the name of the class.

            This one also takes in an attributes vector to allow creating the full
            class object with just the constructor.
        */
        Class(std::string name, std::vector<Attribute> attributes);
        
        /**
            Takes an Attribute object and registers it into the Class
        */
        void registerAttribute(Attribute attribute);
        /**
            Takes the name and type of a new attribute, creates it and
            registers it into the Class
        */
        void registerAttribute(std::string name, type::Type type);

        /**
            Returns the name of the Class
        */
        std::string getName();
        /**
            Returns the names of all registered attributes
        */
        std::vector<std::string> getAttributes();
        /**
            Returns true if the ID corresponds to a registered attribute.
        */
        bool attributeExists(uint32_t id);
        /**
            Returns true if the attribute name corresponds to a registered attribute
        */
        bool attributeExists(std::string attribute);
        /**
            Takes the ID of an attribute as its input and returns
            the name of the attribute
        */
        std::string getAttributeName(uint32_t id);
        /**
            Takes the name of an attribute as its input and returns
            the ID of the attribute
        */
        uint32_t getAttributeID(std::string attribute);
        /**
            Takes the name of an attribute as its input and returns
            the data type of the attribute
        */
        type::Type getAttributeType(std::string attribute);
        /**
            Takes the ID of an attribute as its input and returns
            the data type of the attribute
        */
        type::Type getAttributeType(uint32_t id);

    private:
        // Class name
        std::string name;
        // List of attributes
        std::vector<Attribute> attributes;
        std::vector<std::string> attributesStr;
    };
}