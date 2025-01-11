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
            vec2_t() {
                x = 0;
                y = 0;
            }
            vec2_t(numType xy) {
                x = xy;
                y = xy;
            }
            vec2_t(numType x, numType y) {
                this->x = x;
                this->y = y;
            }

            numType x;
            numType y;
        };
        /**
            Template class for vectors of 3 numbers
        */
        template<class numType>
        class vec3_t {
        public:
            vec3_t() {
                x = 0;
                y = 0;
                z = 0;
            }
            vec3_t(numType xyz) {
                x = xyz;
                y = xyz;
                z = xyz;
            }
            vec3_t(numType x, numType y, numType z) {
                this->x = x;
                this->y = y;
                this->z = z;
            }

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
            vec4_t() {
                x = 0;
                y = 0;
                z = 0;
                w = 0;
            }
            vec4_t(numType xyzw) {
                x = xyzw;
                y = xyzw;
                z = xyzw;
                w = xyzw;
            }
            vec4_t(numType x, numType y, numType z, numType w) {
                this->x = x;
                this->y = y;
                this->z = z;
                this->w = w;
            }

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