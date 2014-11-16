#define BOOST_TEST_MODULE types
#include <boost/test/included/unit_test.hpp>

#include "types.h"

using namespace pgamecc;

BOOST_AUTO_TEST_CASE(types_mod_down) {
    struct {
        int n, d;
        int div, mod, round;
    } v[] = {
        { -6, 3, -2, 0, -6 },
        { -5, 3, -2, 1, -6 },
        { -4, 3, -2, 2, -6 },
        { -3, 3, -1, 0, -3 },
        { -2, 3, -1, 1, -3 },
        { -1, 3, -1, 2, -3 },
        {  0, 3,  0, 0,  0 },
        {  1, 3,  0, 1,  0 },
        {  2, 3,  0, 2,  0 },
        {  3, 3,  1, 0,  3 },
        {  4, 3,  1, 1,  3 },
        {  5, 3,  1, 2,  3 },
        {  6, 3,  2, 0,  6 }
    };

    for (auto t: v) {
        BOOST_CHECK_EQUAL(div_down(t.n, t.d), t.div);
        BOOST_CHECK_EQUAL(mod_down(t.n, t.d), t.mod);
        BOOST_CHECK_EQUAL(round_down(t.n, t.d), t.round);
    }
}
