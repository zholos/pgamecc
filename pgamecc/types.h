#ifndef PGAMECC_TYPES_H
#define PGAMECC_TYPES_H

#include <iostream>
#include <type_traits>

#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace pgamecc {

// these are commonly used

using glm::bvec2;
using glm::bvec3;
using glm::bvec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::dquat;

static_assert(
    sizeof(ivec2::value_type) >= sizeof(int) &&
    sizeof(ivec3::value_type) >= sizeof(int) &&
    sizeof(ivec4::value_type) >= sizeof(int),
    "ivec expected to be at least int, not short");


// useful operators

namespace detail {

template<typename T> inline constexpr
std::enable_if_t<std::is_integral<T>::value, T>
pick_down(T n, T d)
{
    return n < 0 ? d : 0;
}

inline ivec2 pick_down(ivec2 n, ivec2 d) {
    return ivec2(n.x < 0 ? d.x : 0,
                 n.y < 0 ? d.y : 0);
}
inline ivec3 pick_down(ivec3 n, ivec3 d) {
    return ivec3(n.x < 0 ? d.x : 0,
                 n.y < 0 ? d.y : 0,
                 n.z < 0 ? d.z : 0);
}
inline ivec4 pick_down(ivec4 n, ivec4 d) {
    return ivec4(n.x < 0 ? d.x : 0,
                 n.y < 0 ? d.y : 0,
                 n.z < 0 ? d.z : 0,
                 n.w < 0 ? d.w : 0);
}

} // detail

// not constexpr because glm::vec has no constexpr constructor
template<typename T> inline auto
div_down(T n, T d)
{
    // assert(d > 0);
    return (n - detail::pick_down(n, d-1)) / d;
}

template<typename T> inline auto
mod_down(T n, T d)
{
    // assert(d > 0);
    return n % d + detail::pick_down(n % d, d);
}

template<typename T> inline auto
round_down(T n, T d)
{
    return n - mod_down(n, d);
}


// for std::set and std::map
struct ivec2_compare {
    bool operator()(ivec2 a, ivec2 b) const {
        return a.y < b.y || a.y == b.y && a.x < b.x;
    }
};


// glm only implements these for floats (careful with overflow)
inline int dot(ivec2 a, ivec2 b) { return a.x*b.x + a.y*b.y; }
inline int dot(ivec3 a, ivec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline int length2(ivec2 v) { return dot(v, v); }
inline int length2(ivec3 v) { return dot(v, v); }
inline int distance2(ivec2 a, ivec2 b) { return length2(a - b); }
inline int distance2(ivec3 a, ivec3 b) { return length2(a - b); }


// since pgamecc::vec is an alias for glm::detail::tvec, need using for these:
// using namespace pgamecc;
// using pgamecc::operator<<;

inline std::ostream&
operator<<(std::ostream& os, pgamecc::bvec2 v) {
    return os << "bvec2(" << v.x << ", " << v.y << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::bvec3 v) {
    return os << "bvec3(" << v.x << ", " << v.y << ", " << v.z << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::bvec4 v) {
    return os <<
        "bvec4(" << v.x << ", " << v.y <<  ", " << v.z << ", " << v.w << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::ivec2 v) {
    return os << "ivec2(" << v.x << ", " << v.y << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::ivec3 v) {
    return os << "ivec3(" << v.x << ", " << v.y << ", " << v.z << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::ivec4 v) {
    return os <<
        "ivec4(" << v.x << ", " << v.y <<  ", " << v.z << ", " << v.w << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::dvec2 v) {
    return os << "dvec2(" << v.x << ", " << v.y << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::dvec3 v) {
    return os << "dvec3(" << v.x << ", " << v.y <<  ", " << v.z << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::dvec4 v) {
    return os <<
        "dvec4(" << v.x << ", " << v.y <<  ", " << v.z << ", " << v.w << ')';
}

inline std::ostream&
operator<<(std::ostream& os, pgamecc::dquat q) {
    // order matches constructor arguments, not struct fields
    return os <<
        "dquat(" << q.w << ", " << q.x << ", " << q.y <<  ", " << q.z << ')';
}

}

#endif
