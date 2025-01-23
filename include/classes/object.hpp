#pragma once

// eclang
#include "classes/attribute.hpp"
// std
#include <cstdint>
#include <string>
#include <vector>

namespace eclang {
    /**
        A name, class type and array of Attributes.

        Unlike in Classes, this time the attributes have data in
        them that we can obtain easily:

        Example:
        --------------------------------------------------------------
        1 | uint8_t attribute = object->getIDOf("exampleAttribute");  |
        2 |                                                           |
        3 | // Check if attribute is set/exists                       |
        4 | if (attribute == -1) {                                    |
        5 |     return 0;                                             |
        6 | }                                                         |
        7 |                                                           |
        8 | // Check type of attribute                                |
        9 | if (object->getTypeOf(attribute) == eclang::type::FLOAT) {|
        10|     float value = object->getFloatOf(attribute);          |
        11|     // Do something...                                    |
        12| }                                                         |
        --------------------------------------------------------------
    */
    // TODO: Separate the current "Object" objects into internal and API classes
    // The classes should be Object for the internal class and Node for the API
    // The change in name for "nodes" is because the files are supposed to represent
    // a tree.
    class Object {
    public:
        /**
            Constructs the Object object with the class name and the object's name
        */
        Object(std::string className, std::string name, uint8_t sourceFileID);
        /**
            Delete all the registered Attributes
        */
        ~Object();

        /**
            Adds an attribute. This attribute must contain data.
            Do not call this manually.
        */
        void _addAttribute(Attribute* a);
        /**
            Adds an Object object as a child of this object.
            Do not call this manually.
        */
        void _addChild(Object* o);
        /**
            Adds an array of Objects as children of this object.
            Do not call this manually.
        */
        void _addChildren(std::vector<Object*> o);

        /**
            Returns the name of the object
        */
        std::string getName();
        /**
            Returns the name of the class.
        */
        std::string getClassName();
        /**
            Returns the names of all registered attributes
        */
        std::vector<std::string> getAttributes();

        /**
            Returns all children of this object.
            The vector may be empty
        */
        std::vector<Object*> getObjects();
        /**
            Returns the Object objects with the class name specified
            from the current file as a vector.
        */
        std::vector<Object*> getObjectsByClass(std::string className);
        /**
            Returns the Object object with the name specified.
            The pointer returned may be nullptr.
        */
        Object* getObject(std::string name);

        /**
            Returns the ID of the attribute.
            This ID is required to use the other methods
        */
        uint8_t getIDOf(std::string attribute);

        /**
            Returns the data type of this attribute.

            You can use this to check if the attribute is of
            the same type that you need/expect
        */
        type::Type getTypeOf(uint8_t attribute);

        /**
            Returns the source file ID for this object
        */
        uint8_t getSourceFileID();

        
        // Not gonna comment all of this lol
        // GET DATA ------------------------

        // Int
        int8_t getInt8Of(uint8_t attribute);
        int16_t getInt16Of(uint8_t attribute);
        int32_t getInt32Of(uint8_t attribute);
        int64_t getInt64Of(uint8_t attribute);
        // Uint
        uint8_t getUint8Of(uint8_t attribute);
        uint16_t getUint16Of(uint8_t attribute);
        uint32_t getUint32Of(uint8_t attribute);
        uint64_t getUint64Of(uint8_t attribute);
        // Decimal
        float getFloatOf(uint8_t attribute);
        double getDoubleOf(uint8_t attribute);
        // Strings
        std::string getStringOf(uint8_t attribute); // Used for String and Markdown Strings. The only difference between strings and MD strings is how the program interprets them
        // Vectors
        vec2i getVec2iOf(uint8_t attribute); // int
        vec3i getVec3iOf(uint8_t attribute);
        vec4i getVec4iOf(uint8_t attribute);
        vec2l getVec2lOf(uint8_t attribute); // long
        vec3l getVec3lOf(uint8_t attribute);
        vec4l getVec4lOf(uint8_t attribute);
        vec2f getVec2fOf(uint8_t attribute); // float
        vec3f getVec3fOf(uint8_t attribute);
        vec4f getVec4fOf(uint8_t attribute);
        vec2d getVec2dOf(uint8_t attribute); // double
        vec3d getVec3dOf(uint8_t attribute);
        vec4d getVec4dOf(uint8_t attribute);
    private:
        /**
            Throws an error if the number of attributes is too big
        */
        void checkAttributesNumber();


        std::string name; // This object's name
        std::string className; // The class' name
        std::vector<Attribute*> attributes; // List of attributes (with data)
        std::vector<Object*> children; // List of other objects

        
        // ID of the file that loaded this object. Each ID corresponds to a file
        // in the `includedFilenames` array from the EcLang object
        uint8_t sourceFileID;
    };
}