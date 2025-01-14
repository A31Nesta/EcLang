#include "classes/attribute.hpp"
#include "classes/vectors.hpp"
#include <cstdint>
#include <iostream>
#include <string>

namespace eclang {
    // CONSTRUCTORS
    // ------------

    /**
        Basic constructor. When defining a new language we use this constructor.
        It specifies the name and the type but no value.
    */
    Attribute::Attribute(std::string name, type::Type type) {
        this->name = name;
        this->type = type;
    }

    /**
        Typed constructors. When reading a file, the values of the attributes are
        stored using this constructor.
    */
    Attribute::Attribute(std::string name, int8_t value) {
        this->name = name;
        this->type = type::INT8;
        bytesValue = new int8_t(value); // copy value
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, int16_t value) {
        this->name = name;
        this->type = type::INT16;
        bytesValue = new int16_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, int32_t value) {
        this->name = name;
        this->type = type::INT32;
        bytesValue = new int32_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, int64_t value) {
        this->name = name;
        this->type = type::INT64;
        bytesValue = new int64_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, uint8_t value) {
        this->name = name;
        this->type = type::UINT8;
        bytesValue = new uint8_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, uint16_t value) {
        this->name = name;
        this->type = type::UINT16;
        bytesValue = new uint16_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, uint32_t value) {
        this->name = name;
        this->type = type::UINT32;
        bytesValue = new uint32_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, uint64_t value) {
        this->name = name;
        this->type = type::UINT64;
        bytesValue = new uint64_t(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, float value) {
        this->name = name;
        this->type = type::FLOAT;
        bytesValue = new float(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, double value) {
        this->name = name;
        this->type = type::DOUBLE;
        bytesValue = new double(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, std::string value, type::Type type) {
        this->name = name;
        this->type = type;
        bytesValue = new std::string(value);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec2i value) {
        this->name = name;
        this->type = type::VEC2I;
        bytesValue = new vec2i(value.x, value.y);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec3i value) {
        this->name = name;
        this->type = type::VEC3I;
        bytesValue = new vec3i(value.x, value.y, value.z);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec4i value) {
        this->name = name;
        this->type = type::VEC4I;
        bytesValue = new vec4i(value.x, value.y, value.z, value.w);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec2l value) {
        this->name = name;
        this->type = type::VEC2L;
        bytesValue = new vec2l(value.x, value.y);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec3l value) {
        this->name = name;
        this->type = type::VEC3L;
        bytesValue = new vec3l(value.x, value.y, value.z);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec4l value) {
        this->name = name;
        this->type = type::VEC4L;
        bytesValue = new vec4l(value.x, value.y, value.z, value.w);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec2f value) {
        this->name = name;
        this->type = type::VEC2F;
        bytesValue = new vec2f(value.x, value.y);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec3f value) {
        this->name = name;
        this->type = type::VEC3F;
        bytesValue = new vec3f(value.x, value.y, value.z);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec4f value) {
        this->name = name;
        this->type = type::VEC4F;
        bytesValue = new vec4f(value.x, value.y, value.z, value.w);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec2d value) {
        this->name = name;
        this->type = type::VEC2D;
        bytesValue = new vec2d(value.x, value.y);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec3d value) {
        this->name = name;
        this->type = type::VEC3D;
        bytesValue = new vec3d(value.x, value.y, value.z);
        sizeofValue = sizeof(value);
    }
    Attribute::Attribute(std::string name, vec4d value) {
        this->name = name;
        this->type = type::VEC4D;
        bytesValue = new vec4d(value.x, value.y, value.z, value.w);
        sizeofValue = sizeof(value);
    }

    /**
        Destructor. The way data is stored is sussy so we do
        need a destructor
    */
    Attribute::~Attribute() {
        // If size of value is not 0 we have a value
        // if we have a value we have to delete it
        if (sizeofValue != 0) {
            if (bytesValue == nullptr) {
                std::cerr << "sizeofValue was 0 but bytesValue was nullptr. I don't know how this is possible but memory may have been leaked.\n";
                return;
            }
            // Delete value
            switch (type) {
            case type::INT8:
                delete (int8_t*)bytesValue;
                break;
            case type::INT16:
                delete (int16_t*)bytesValue;
                break;
            case type::INT32:
                delete (int32_t*)bytesValue;
                break;
            case type::INT64:
                delete (int64_t*)bytesValue;
                break;
            case type::UINT8:
                delete (uint8_t*)bytesValue;
                break;
            case type::UINT16:
                delete (uint16_t*)bytesValue;
                break;
            case type::UINT32:
                delete (uint32_t*)bytesValue;
                break;
            case type::UINT64:
                delete (uint64_t*)bytesValue;
                break;
            case type::FLOAT:
                delete (float*)bytesValue;
                break;
            case type::DOUBLE:
                delete (double*)bytesValue;
                break;
            case type::STRING:
            case type::STR_MD:
                delete (std::string*)bytesValue;
                break;
            case type::VEC2I:
                delete (vec2i*)bytesValue;
                break;
            case type::VEC3I:
                delete (vec3i*)bytesValue;
                break;
            case type::VEC4I:
                delete (vec4i*)bytesValue;
                break;
            case type::VEC2L:
                delete (vec2l*)bytesValue;
                break;
            case type::VEC3L:
                delete (vec3l*)bytesValue;
                break;
            case type::VEC4L:
                delete (vec4l*)bytesValue;
                break;
            case type::VEC2F:
                delete (vec2f*)bytesValue;
                break;
            case type::VEC3F:
                delete (vec3f*)bytesValue;
                break;
            case type::VEC4F:
                delete (vec4f*)bytesValue;
                break;
            case type::VEC2D:
                delete (vec2d*)bytesValue;
                break;
            case type::VEC3D:
                delete (vec3d*)bytesValue;
                break;
            case type::VEC4D:
                delete (vec4d*)bytesValue;
                break;
            }
        }
    }

    // Get Name
    // --------
    std::string Attribute::getName() {
        return name;
    }

    // Get Type
    // --------
    type::Type Attribute::getType() {
        return type;
    }

    // Get Value
    // ---------
    // Int
    int8_t Attribute::getInt8() {
        return *(int8_t*)bytesValue;
    }
    int16_t Attribute::getInt16() {
        return *(int16_t*)bytesValue;
    }
    int32_t Attribute::getInt32() {
        return *(int32_t*)bytesValue;
    }
    int64_t Attribute::getInt64() {
        return *(int64_t*)bytesValue;
    }
    // Uint
    uint8_t Attribute::getUint8() {
        return *(uint8_t*)bytesValue;
    }
    uint16_t Attribute::getUint16() {
        return *(uint16_t*)bytesValue;
    }
    uint32_t Attribute::getUint32() {
        return *(uint32_t*)bytesValue;
    }
    uint64_t Attribute::getUint64() {
        return *(uint64_t*)bytesValue;
    }
    // Decimal
    float Attribute::getFloat() {
        return *(float*)bytesValue;
    }
    double Attribute::getDouble() {
        return *(double*)bytesValue;
    }
    // Strings
    // Used for String and Markdown Strings. The only difference between strings and MD strings is how the program interprets them
    std::string Attribute::getString() {
        return *(std::string*)bytesValue;
    }
    // Vectors
    // int
    vec2i Attribute::getVec2i() {
        return *(vec2i*)bytesValue;
    }
    vec3i Attribute::getVec3i() {
        return *(vec3i*)bytesValue;
    }
    vec4i Attribute::getVec4i() {
        return *(vec4i*)bytesValue;
    }
    // long
    vec2l Attribute::getVec2l() {
        return *(vec2l*)bytesValue;
    }
    vec3l Attribute::getVec3l() {
        return *(vec3l*)bytesValue;
    }
    vec4l Attribute::getVec4l() {
        return *(vec4l*)bytesValue;
    }
    // float
    vec2f Attribute::getVec2f() {
        return *(vec2f*)bytesValue;
    }
    vec3f Attribute::getVec3f() {
        return *(vec3f*)bytesValue;
    }
    vec4f Attribute::getVec4f() {
        return *(vec4f*)bytesValue;
    }
    // double
    vec2d Attribute::getVec2d() {
        return *(vec2d*)bytesValue;
    }
    vec3d Attribute::getVec3d() {
        return *(vec3d*)bytesValue;
    }
    vec4d Attribute::getVec4d() {
        return *(vec4d*)bytesValue;
    }
}