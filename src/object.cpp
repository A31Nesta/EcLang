#include "classes/object.hpp"
#include "classes/attribute.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace eclang {
    /**
        Constructs the Object object with the class name and the object's name
    */
    Object::Object(std::string className, std::string name) {
        // Copy data to object
        this->className = className;
        this->name = name;
    }

    /**
        Adds an attribute. This attribute must contain data.
        Do not call this manually.
    */
    void Object::_addAttribute(Attribute a) {
        attributes.push_back(a);
    }
    /**
        Adds an Object object as a child of this object.
        Do not call this manually.
    */
    void Object::_addChild(Object o) {
        children.push_back(o);
    }

    /**
        Returns the name of the object
    */
    std::string Object::getName() {
        return name;
    }
    /**
        Returns the name of the class.
    */
    std::string Object::getClassName() {
        return className;
    }

    /**
        Returns all children of this object.
        The vector may be empty
    */
    std::vector<Object> Object::getChildren() {
        return children;
    }

    /**
        Returns the ID of the attribute.
        This ID is required to use the other methods
    */
    uint8_t Object::getIDOf(std::string attribute) {
        // If for whatever reason we have too many attributes throw error
        checkAttributesNumber();

        // Find attribute with this name. If we find it, return the ID
        for (uint8_t i = 0; i < attributes.size(); i++) {
            Attribute a = attributes.at(i);
            if (a.getName() == attribute) {
                return i;
            }
        }
        // If we didn't find it, return invalid ID
        return -1;
    }

    /**
        Returns the data type of this attribute.

        You can use this to check if the attribute is of
        the same type that you need/expect
    */
    type::Type Object::getTypeOf(uint8_t attribute) {
        return attributes.at(attribute).getType();
    }

    
    // Not gonna comment all of this lol
    // GET DATA ------------------------

    // Int
    int8_t Object::getInt8Of(uint8_t attribute) {
        return attributes.at(attribute).getInt8();
    }
    int16_t Object::getInt16Of(uint8_t attribute) {
        return attributes.at(attribute).getInt16();
    }
    int32_t Object::getInt32Of(uint8_t attribute) {
        return attributes.at(attribute).getInt32();
    }
    int64_t Object::getInt64Of(uint8_t attribute) {
        return attributes.at(attribute).getInt64();
    }
    // Uint
    uint8_t Object::getUint8Of(uint8_t attribute) {
        return attributes.at(attribute).getUint8();
    }
    uint16_t Object::getUint16Of(uint8_t attribute) {
        return attributes.at(attribute).getUint16();
    }
    uint32_t Object::getUint32Of(uint8_t attribute) {
        return attributes.at(attribute).getUint32();
    }
    uint64_t Object::getUint64Of(uint8_t attribute) {
        return attributes.at(attribute).getUint64();
    }
    // Decimal
    float Object::getFloatOf(uint8_t attribute) {
        return attributes.at(attribute).getFloat();
    }
    double Object::getDoubleOf(uint8_t attribute) {
        return attributes.at(attribute).getDouble();
    }
    // Strings
    std::string Object::getStringOf(uint8_t attribute) { // Used for String and Markdown Strings. The only difference between strings and MD strings is how the program interprets them
        return attributes.at(attribute).getString();
    }
    // Vectors
    vec2i Object::getVec2iOf(uint8_t attribute) { // int
        return attributes.at(attribute).getVec2i();
    }
    vec3i Object::getVec3iOf(uint8_t attribute) {
        return attributes.at(attribute).getVec3i();
    }
    vec4i Object::getVec4iOf(uint8_t attribute) {
        return attributes.at(attribute).getVec4i();
    }
    vec2l Object::getVec2lOf(uint8_t attribute) { // long
        return attributes.at(attribute).getVec2l();
    }
    vec3l Object::getVec3lOf(uint8_t attribute) {
        return attributes.at(attribute).getVec3l();
    }
    vec4l Object::getVec4lOf(uint8_t attribute) {
        return attributes.at(attribute).getVec4l();
    }
    vec2f Object::getVec2fOf(uint8_t attribute) { // float
        return attributes.at(attribute).getVec2f();
    }
    vec3f Object::getVec3fOf(uint8_t attribute) {
        return attributes.at(attribute).getVec3f();
    }
    vec4f Object::getVec4fOf(uint8_t attribute) {
        return attributes.at(attribute).getVec4f();
    }
    vec2d Object::getVec2dOf(uint8_t attribute) { // double
        return attributes.at(attribute).getVec2d();
    }
    vec3d Object::getVec3dOf(uint8_t attribute) {
        return attributes.at(attribute).getVec3d();
    }
    vec4d Object::getVec4dOf(uint8_t attribute) {
        return attributes.at(attribute).getVec4d();
    }


    // PRIVATE
    // -------
    /**
        Throws an error if the number of attributes is too big
    */
    void Object::checkAttributesNumber() {
        if (attributes.size() > UINT8_MAX) {
            throw std::runtime_error(
                "ECLANG_ERROR: Too many attributes found on \""+name+"\" class."
                "("+std::to_string(attributes.size())+" attributes. Maximum is "+std::to_string(UINT8_MAX)+")"
            );
        }
    }
}