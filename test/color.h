#include "types.h"

#include <pgamecc/color.h>

#include <iostream>

using std::ostream;
using namespace pgamecc;
using namespace pgamecc::color;


// equivalence predicate

template<> inline bool
eq(RGB x, RGB y) {
    return eq(x.r, y.r) && eq(x.g, y.g) && eq(x.b, y.b);
}

template<> inline bool
eq(sRGB x, sRGB y) {
    return eq(x.r, y.r) && eq(x.g, y.g) && eq(x.b, y.b);
}

template<> inline bool
eq(HSL x, HSL y) {
    return eq(x.h, y.h) && eq(x.s, y.s) && eq(x.l, y.l);
}

template<> inline bool
eq(YCH x, YCH y) {
    return eq(x.y, y.y) && eq(x.c, y.c) && eq(x.h, y.h);
}
