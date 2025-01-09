#pragma once

// std
#include <cstdint>

namespace eclang {
    namespace _template {
        /**
            Template class for vectors of 2 numbers
        */
        template<class numType>
        class vec2_t {
        public:
            vec2_t();
            vec2_t(numType xy);
            vec2_t(numType x, numType y);

            numType x;
            numType y;
        };
        /**
            Template class for vectors of 3 numbers
        */
        template<class numType>
        class vec3_t {
        public:
            vec3_t();
            vec3_t(numType xyz);
            vec3_t(numType x, numType y, numType z);

            numType x;
            numType y;
            numType z;
        };
        /**
            Template class for vectors of 4 numbers
        */
        template<class numType>
        class vec4_t {
        public:
            vec4_t();
            vec4_t(numType xyzw);
            vec4_t(numType x, numType y, numType z, numType w);

            numType x;
            numType y;
            numType z;
            numType w;
        };
    }

    // Typedefs
    typedef _template::vec2_t<int32_t> vec2i;
    typedef _template::vec3_t<int32_t> vec3i;
    typedef _template::vec4_t<int32_t> vec4i;

    typedef _template::vec2_t<int64_t> vec2l;
    typedef _template::vec3_t<int64_t> vec3l;
    typedef _template::vec4_t<int64_t> vec4l;

    typedef _template::vec2_t<float> vec2f;
    typedef _template::vec3_t<float> vec3f;
    typedef _template::vec4_t<float> vec4f;

    typedef _template::vec2_t<double> vec2d;
    typedef _template::vec3_t<double> vec3d;
    typedef _template::vec4_t<double> vec4d;
}