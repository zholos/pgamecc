#include <pgamecc/loc.h>

#include <algorithm>

#include <glm/gtx/component_wise.hpp>

using std::abs;

namespace pgamecc {

static inline void
check_irot_axes(ivec3 a) {
    bool have[3] = {};
    for (auto i: { a.x, a.y, a.z }) {
        assert(-3 <= i && i <= 3 && i != 0);
        if (have[abs(i)-1]++)
            assert(false);
    }
}

inline ivec3
axes(irot r) {
    ivec3 a = r * ivec3(1, 2, 3);
    check_irot_axes(a);
    return a;
}

inline irot
irot_from_axes(ivec3 a) {
    check_irot_axes(a);
    irot r;
    while (abs(axes(r).x) != abs(a.x))
        r = irot::rotate_xyz() * r;
    while (abs(axes(r).y) != abs(a.y))
        r = irot::rotate_x() * r;
    if (axes(r).x != a.x)
        r = irot::flip_x() * r;
    if (axes(r).y != a.y)
        r = irot::flip_y() * r;
    if (axes(r).z != a.z)
        r = irot::flip_z() * r;
    assert(axes(r) == a);
    return r;
}

inline ivec3
int_dvec3(dvec3 d) {
    ivec3 i(glm::round(d));
    assert(glm::compMax(glm::abs(dvec3(i)-d)) < .1);
    return i;
}

inline irot
irot_from_quat(dquat q) {
    return irot_from_axes(int_dvec3(q * dvec3(1, 2, 3)));
}

inline bool
operator<(irot a_, irot b_) {
    auto a = axes(a_), b = axes(b_);
    return a.x < b.x || a.x == b.x && (a.y < b.y || a.y == b.y && a.z < b.z);
}

}
