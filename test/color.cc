#define BOOST_TEST_MODULE color
#include <boost/test/included/unit_test.hpp>

#include "color.h"

using namespace pgamecc::color;


//
// The tests
//

BOOST_AUTO_TEST_CASE(color_test_test) {
    RGB rgb[4] = {
        { .5, .5, .5 },
        { .499, .5, .5 },
        { .5, .499, .5 },
        { .5, .5, .499 }
    };
    BOOST_REQUIRE(eq(rgb[0], rgb[0]));
    BOOST_REQUIRE(!eq(rgb[0], rgb[1]));
    BOOST_REQUIRE(!eq(rgb[0], rgb[2]));
    BOOST_REQUIRE(!eq(rgb[0], rgb[3]));
}

BOOST_AUTO_TEST_CASE(color_rgb) {
    RGB  rgb[2] = {
        { 0, .001, .2 },
        { .4479884124418831613988088, .6920710568653717757419120, 1 }
    };
    sRGB srgb[2] = {
        { 0, .01292, .4845292044817069480900069 },
        { .7, .85, 1 }
    };
    CHECK_EQ(rgb[0].srgb(), srgb[0]);
    CHECK_EQ(rgb[1].srgb(), srgb[1]);
    CHECK_EQ(rgb[0], srgb[0].rgb());
    CHECK_EQ(rgb[1], srgb[1].rgb());
}
