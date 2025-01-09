#pragma once

// eclang
#include "classes/vectors.hpp"
// std
#include <cstdint>
#include <string>

namespace eclang::language {
    namespace type {
        /**
            All possible types in EcLang

            TODO: Proper documentation for data types in EcLang
        */
        enum Type {
            INT8,   // -> (byte, char, int8, int8_t)
            INT16,  // -> (short, int16, int16_t)
            INT32,  // -> (int, int32, int32_t)
            INT64,  // -> (long, int64, int64_t)

            UINT8,  // -> (uint8, uint8_t)
            UINT16, // -> (uint16, uint16_t)
            UINT32, // -> (uint32, uint32_t)
            UINT64, // -> (uint64, uint64_t)

            FLOAT,  // -> (float)
            DOUBLE, // -> (double)

            STRING, // -> (string)
            STR_MD, // -> (str_md) // a string that should be

            VEC2I,  // -> (vec2i) // a vector of two signed 3
            VEC3I,  // -> (vec3i) // a vector of three signed
            VEC4I,  // -> (vec4i) // a vector of four signed 

            VEC2L,  // -> (vec2l) // a vector of two signed 6
            VEC3L,  // -> (vec3l) // a vector of three signed
            VEC4L,  // -> (vec4l) // a vector of four signed 

            VEC2F,  // -> (vec2, vec2f) // a vector of two fl
            VEC3F,  // -> (vec3, vec3f) // a vector of three 
            VEC4F,  // -> (vec4, vec4f) // a vector of four f

            VEC2D,  // -> (vec2d) // a vector of two doubles
            VEC3D,  // -> (vec3d) // a vector of three double
            VEC4D   // -> (vec4d) // a vector of four doubles
        };
    }

    /**
        An attribute of a Class in EcLang.
        An attribute consists of a name, a data
        type and, optionally, the value.

        This class is used in the Class and Object classes.
        In "Class" it's used to define a language, so only the name and type are used.
        In "Object" it's used to describe the data in a file, so the value is also used
    */
    class Attribute {
    public:
        /**
            Basic constructor. When defining a new language we use this constructor.
            It specifies the name and the type but no value.
        */
        Attribute(std::string name, type::Type type);
        /**
            Typed constructors. When reading a file, the values of the attributes are
            stored using this constructor.
        */
        Attribute(std::string name, int8_t value); // int
        Attribute(std::string name, int16_t value);
        Attribute(std::string name, int32_t value);
        Attribute(std::string name, int64_t value);
        Attribute(std::string name, uint8_t value); // uint
        Attribute(std::string name, uint16_t value);
        Attribute(std::string name, uint32_t value);
        Attribute(std::string name, uint64_t value);
        Attribute(std::string name, float value); // decimals
        Attribute(std::string name, double value);
        Attribute(std::string name, std::string value, type::Type type); // strings; type is required because a string can be normal or MD
        Attribute(std::string name, vec2i value);
        Attribute(std::string name, vec3i value);
        Attribute(std::string name, vec4i value);
        Attribute(std::string name, vec2l value);
        Attribute(std::string name, vec3l value);
        Attribute(std::string name, vec4l value);
        Attribute(std::string name, vec2f value);
        Attribute(std::string name, vec3f value);
        Attribute(std::string name, vec4f value);
        Attribute(std::string name, vec2d value);
        Attribute(std::string name, vec3d value);
        Attribute(std::string name, vec4d value);

        /**
            Destructor. The way data is stored is sussy so we do
            need a destructor
        */
        ~Attribute();

        // Get Name
        // --------
        std::string getName();

        // Get Type
        // --------
        type::Type getType();

        // Get Value
        // ---------
        // Int
        int8_t getInt8();
        int16_t getInt16();
        int32_t getInt32();
        int64_t getInt64();
        // Uint
        uint8_t getUint8();
        uint16_t getUint16();
        uint32_t getUint32();
        uint64_t getUint64();
        // Decimal
        float getFloat();
        double getDouble();
        // Strings
        std::string getString(); // Used for String and Markdown Strings. The only difference between strings and MD strings is how the program interprets them
        // Vectors
        vec2i getVec2i(); // int
        vec3i getVec3i();
        vec4i getVec4i();
        vec2l getVec2l(); // long
        vec3l getVec3l();
        vec4l getVec4l();
        vec2f getVec2f(); // float
        vec3f getVec3f();
        vec4f getVec4f();
        vec2d getVec2d(); // double
        vec3d getVec3d();
        vec4d getVec4d();
    private:
        // Name of the attribute
        std::string name;
        // Type of the attribute
        type::Type type;

        // Value
        const void* bytesValue = nullptr; // IMPORTANT: DELETE!
        uint8_t sizeofValue = 0; // if this is more than 0, delete bytesValue in destructor
    };
}