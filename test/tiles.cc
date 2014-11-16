#define BOOST_TEST_MODULE tiles
#include <boost/test/included/unit_test.hpp>

#include "tiles.h"
#include "types.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>

using std::set;
using std::inserter;
using std::generate_n;
using std::domain_error;
using std::min;
using std::max;
using std::abs;
using std::map;
using std::sqrt;

using namespace pgamecc;


//
// The tests
//

// help boost print_log_value find it
namespace glm { namespace detail {
using pgamecc::operator<<;
}}

BOOST_AUTO_TEST_CASE(tiles_hex) {
    HexTiling t;

    // see the illustration in tiles.h
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 0), ivec2( 1,  0));
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 1), ivec2( 0,  1));
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 2), ivec2(-1,  1));
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 3), ivec2(-1,  0));
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 4), ivec2( 0, -1));
    BOOST_CHECK_EQUAL(t.next(ivec2{}, 5), ivec2( 1, -1));

    for (int i = 0; i < 6; i++) {
        BOOST_CHECK_EQUAL(t.next(ivec2{}, i-6), t.next(ivec2{}, i));
        BOOST_CHECK_EQUAL(t.next(ivec2{}, i+6), t.next(ivec2{}, i));

        BOOST_CHECK_EQUAL(t.next(ivec2{}, i, 0), ivec2{});
        BOOST_CHECK_EQUAL(t.next(ivec2{}, i, 3), t.next(ivec2{}, i) * 3);
    }

    BOOST_CHECK_EQUAL(t.dist(ivec2{2, -1}, ivec2{2, -1}), 0);
    for (int i = 0; i < 6; i++) {
        BOOST_CHECK_EQUAL(t.dist(t.next(ivec2{}, i), ivec2{}), 1);
        BOOST_CHECK_EQUAL(t.dist(t.next(ivec2{}, i), t.next(ivec2{}, i+1)), 1);
        BOOST_CHECK_EQUAL(t.dist(t.next(ivec2{}, i), t.next(ivec2{}, i+2)), 2);
        BOOST_CHECK_EQUAL(t.dist(t.next(ivec2{}, i), t.next(ivec2{}, i+3)), 2);
    }

    int disk_size = 0;
    for (int i = 0; i <= 5; i++) {
        set<ivec2, ivec2_compare> circle;
        for (auto c: t.circle(ivec2{1, -1}, i)) {
            BOOST_CHECK_EQUAL(t.dist(ivec2{1, -1}, c), i);
            circle.insert(c);
        }
        int circle_size = i ? 6*i : 1;
        BOOST_CHECK_EQUAL(circle.size(), circle_size);

        set<ivec2, ivec2_compare> disk;
        for (auto c: t.disk(ivec2{0, 0}, i)) {
            BOOST_CHECK_LE(t.dist(ivec2{0, 0}, c), i);
            disk.insert(c);
        }
        disk_size += circle_size;
        BOOST_CHECK_EQUAL(disk.size(), disk_size);
    }

    BOOST_CHECK_EQUAL(t.to_cartesian(ivec2{1, 1}), dvec2(1.5*sqrt(3), 1.5));
    BOOST_CHECK_EQUAL(t.to_cartesian(ivec2{-2, 2}), dvec2(-sqrt(3), 3));
    BOOST_CHECK_EQUAL(t.to_cartesian(ivec2{1, -2}), dvec2(0, -3));

    dvec2 coords;
    for (coords.y = -5; coords.y <= 5; coords.y += .1)
        for (coords.x = -5; coords.x <= 5; coords.x += .1) {
            ivec2 center = t.from_cartesian(coords);
            for (int i = 0; i < 6; i++)
                BOOST_CHECK_LE(
                    glm::length(coords-t.to_cartesian(center)),
                    glm::length(coords-t.to_cartesian(t.next(center, i))));
        }
}
