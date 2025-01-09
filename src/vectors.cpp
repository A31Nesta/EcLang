#include "classes/vectors.hpp"

namespace eclang::_template {
    // Vec2
    template<class numType> vec2_t<numType>::vec2_t() {
        x = 0;
        y = 0;
    }
    template<class numType> vec2_t<numType>::vec2_t(numType xy) {
        x = xy;
        y = xy;
    }
    template<class numType> vec2_t<numType>::vec2_t(numType x, numType y) {
        this->x = x;
        this->y = y;
    }
    // Vec3
    template<class numType> vec3_t<numType>::vec3_t() {
        x = 0;
        y = 0;
        z = 0;
    }
    template<class numType> vec3_t<numType>::vec3_t(numType xyz) {
        x = xyz;
        y = xyz;
        z = xyz;
    }
    template<class numType> vec3_t<numType>::vec3_t(numType x, numType y, numType z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    // Vec4
    template<class numType> vec4_t<numType>::vec4_t() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }
    template<class numType> vec4_t<numType>::vec4_t(numType xyzw) {
        x = xyzw;
        y = xyzw;
        z = xyzw;
        w = xyzw;
    }
    template<class numType> vec4_t<numType>::vec4_t(numType x, numType y, numType z, numType w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
}