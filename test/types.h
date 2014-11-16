#include <pgamecc/types.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include <boost/test/predicate_result.hpp>

using namespace pgamecc;


// equivalence predicate

template<typename T> inline bool eq(T, T);

struct {
    template<typename T>
    boost::test_tools::predicate_result
    operator()(T a, T b) {
        if (eq(a, b))
            return true;
        else {
            boost::test_tools::predicate_result result(false);
            result.message() << "[" << a << " != " << b << "]";
            return result;
        }
    }
} eq_predicate;

#define CHECK_EQ(a, b) BOOST_CHECK_PREDICATE(eq_predicate, ((a))((b)))


template<> inline bool
eq(double x, double y) {
    constexpr auto prec = 100 * std::numeric_limits<double>::epsilon();
    return std::fabs(x - y) <=
        prec * std::max(1., std::max(std::fabs(x), std::fabs(x)));
}

template<> inline bool
eq(dvec2 a, dvec2 b) {
    return eq(a.x, b.x) && eq(a.y, b.y);
}

template<> inline bool
eq(dquat a, dquat b) {
    return eq(a.x,  b.x) && eq(a.y,  b.y) && eq(a.z,  b.z) && eq(a.w,  b.w) ||
           eq(a.x, -b.x) && eq(a.y, -b.y) && eq(a.z, -b.z) && eq(a.w, -b.w);
}
