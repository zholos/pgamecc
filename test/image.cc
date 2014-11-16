#define BOOST_TEST_MODULE image
#include <boost/test/included/unit_test.hpp>

#include "image.h"

#include "color.h"

using std::logic_error;
using std::out_of_range;

using namespace pgamecc;
using namespace pgamecc::color;


//
// The tests
//

BOOST_AUTO_TEST_CASE(image_gradient) {
    Gradient<RGB> g0;

    CHECK_EQ(g0(0), RGB());

    RGB rgb[4] = { { 1, 0, 0 }, { 0, .5, 1 }, { 1, 1, 1 }, { -1, 0, 1 } };

    auto g1 = g0;
    g1[.5] = rgb[1];

    CHECK_EQ(g1(.25), rgb[1]);
    CHECK_EQ(g1(0),   rgb[1]);
    CHECK_EQ(g1(1),   rgb[1]);

    CHECK_EQ(g0(1),   RGB());

    auto g2 = g1;
    g2[0] = rgb[0];

    CHECK_EQ(g2(-1),  rgb[0]);
    CHECK_EQ(g2(0),   rgb[0]);
    CHECK_EQ(g2(.2), (RGB{ .6, .2, .4 }));
    CHECK_EQ(g2(.5),  rgb[1]);
    CHECK_EQ(g2(1),   rgb[1]);

    CHECK_EQ(g1(.2),  rgb[1]);
    CHECK_EQ(g0(.2),  RGB());

    auto g3 = g2;
    g3[.75] = rgb[2];

    CHECK_EQ(g3(.2), (RGB{ .6, .2, .4 }));
    CHECK_EQ(g3(.5),  rgb[1]);
    CHECK_EQ(g3(.7), (RGB{ .8, .9, 1 }));
    CHECK_EQ(g3(.75), rgb[2]);
    CHECK_EQ(g3(1),   rgb[2]);

    CHECK_EQ(g2(.7),  rgb[1]);
    CHECK_EQ(g1(.7),  rgb[1]);
    CHECK_EQ(g0(.7),  RGB());

    auto g4 = g3;
    g4[.75] = rgb[3]; // overwrite old value

    CHECK_EQ(g4(.2), (RGB{ .6, .2, .4 }));
    CHECK_EQ(g4(.5),  rgb[1]);
    CHECK_EQ(g4(.7), (RGB{ -.8, .1, 1 }));
    CHECK_EQ(g4(.75), rgb[3]);
    CHECK_EQ(g4(1),   rgb[3]);

    CHECK_EQ(g3(.7), (RGB{ .8, .9, 1 }));
    CHECK_EQ(g2(.7),  rgb[1]);
    CHECK_EQ(g1(.7),  rgb[1]);
    CHECK_EQ(g0(.7),  RGB());
}


BOOST_AUTO_TEST_CASE(image_image) {
    Image<RGB> m(ivec2{3, 2});

    BOOST_CHECK_EQUAL(m.size().x, 3);
    BOOST_CHECK_EQUAL(m.size().y, 2);
    CHECK_EQ(m[ivec2(1, 1)], RGB());

    m[ivec2{1, 0}] = RGB{ 1, 0, .3 };
    m[ivec2{2, 0}].g = .75;
    m[ivec2{2, 1}].b = .7;

    CHECK_EQ(m[ivec2(1, 0)], (RGB{ 1, 0, .3 }));
    CHECK_EQ(m[ivec2(2, 0)], (RGB{ 0, .75, 0 }));
    CHECK_EQ(m[ivec2(2, 1)], (RGB{ 0, 0, .7 }));
    CHECK_EQ(m[ivec2(1, 1)],  RGB());

    BOOST_CHECK_THROW(m[ivec2(-1, 0)], out_of_range);
    BOOST_CHECK_THROW(m[ivec2(0, -1)], out_of_range);
    BOOST_CHECK_THROW(m[m.size() * ivec2(1, 0)], out_of_range);
    BOOST_CHECK_THROW(m[m.size() * ivec2(0, 1)], out_of_range);
    BOOST_CHECK_THROW(m[m.size()], out_of_range);

    auto l = m.linear();

    CHECK_EQ(l(dvec2(2. /3, 1./2)), (RGB{ .25, .1875, .25 }));
    CHECK_EQ(l(dvec2(2.3/3, .7/2)), (RGB{ .16, .48, .16 }));
    CHECK_EQ(l(dvec2(2.3/3, 10)),   (RGB{ 0, 0, .56 }));
}


BOOST_AUTO_TEST_CASE(image_render) {
    auto m = make_image(ivec2{10, 5},
        [](dvec2 p) { return RGB{ p.x + p.y, p.x - p.y, p.x * p.y }; });

    BOOST_CHECK_EQUAL(m.size().x, 10);
    BOOST_CHECK_EQUAL(m.size().y, 5);

    auto l = m.linear();

    CHECK_EQ(l(dvec2(.3, .2)), (RGB{  .5,  .1, .06 }));
    CHECK_EQ(l(dvec2(.8, .3)), (RGB{ 1.1,  .5, .24 }));
    CHECK_EQ(l(dvec2(.5, .7)), (RGB{ 1.2, -.2, .35 }));

    auto n = m.apply([](RGB c) { return c.srgb(); });

    CHECK_EQ(n[ivec2(5, 3)], m[ivec2(5, 3)].srgb());
}
