#define BOOST_TEST_MODULE loc
#include <boost/test/included/unit_test.hpp>

#include "loc.h"

#include "types.h"

#include <cstdlib>
#include <list>
#include <set>
#include <utility>

using std::rand;
using std::list;
using std::pair;
using std::set;

using namespace pgamecc;


//
// The tests
//

// help boost print_log_value find it
namespace glm { namespace detail {
using pgamecc::operator<<;
}}


BOOST_AUTO_TEST_CASE(ioct_test) {
    BOOST_CHECK_EQUAL(ioct(0).i(), 0);
    BOOST_CHECK_EQUAL(ioct(7).i(), 7);

    BOOST_CHECK_EQUAL(ioct(5).bvec3_cast(), bvec3(1, 0, 1));
    BOOST_CHECK_EQUAL((~ioct(5)).bvec3_cast(), bvec3(0, 1, 0));

    int i = 0;
    for (auto b: ioct::all())
        BOOST_CHECK_EQUAL(b.i(), i++);
    BOOST_CHECK_EQUAL(i, 8);
}


BOOST_AUTO_TEST_CASE(irot_constructor_test) {
    // These are essentially tautologies at the moment, but will be useful when
    // the internal representation of irot changes.
    BOOST_CHECK_EQUAL(axes(irot()), ivec3(1, 2, 3));
    BOOST_CHECK_EQUAL(axes(irot::rotate_x()), ivec3(1, -3, 2));
    BOOST_CHECK_EQUAL(axes(irot::rotate_y()), ivec3(3, 2, -1));
    BOOST_CHECK_EQUAL(axes(irot::rotate_z()), ivec3(-2, 1, 3));
    BOOST_CHECK_EQUAL(axes(irot::rotate_xyz()), ivec3(3, 1, 2));
    BOOST_CHECK_EQUAL(axes(irot::flip_x()), ivec3(-1, 2, 3));
    BOOST_CHECK_EQUAL(axes(irot::flip_y()), ivec3(1, -2, 3));
    BOOST_CHECK_EQUAL(axes(irot::flip_z()), ivec3(1, 2, -3));
}

static size_t count_group(set<irot> s) {
    size_t count;
    do {
        count = s.size();
        for (auto a: s)
            for (auto b: s)
                s.insert(a * b);
    } while (s.size() != count);
    return count;
}

BOOST_AUTO_TEST_CASE(irot_count_test) {
    set<irot> rotations = {
        irot(),
        irot::rotate_x(),
        irot::rotate_y(),
        irot::rotate_z(),
        irot::rotate_xyz(),
    };
    set<irot> rotoreflections = rotations;
    rotoreflections.insert({
        irot::flip_x(),
        irot::flip_y(),
        irot::flip_z()
    });
    BOOST_CHECK_EQUAL(count_group(rotations), 24);
    BOOST_CHECK_EQUAL(count_group(rotoreflections), 48);

}

static const auto rotations = []{
    list<pair<irot, dquat>> r, r_copy;
    for (int i = 0; i < 8; i++) {
        r.emplace_back(
            irot::rotate_x(i),
            glm::angleAxis(glm::radians(90.*i), dvec3(1, 0, 0)));
        r.emplace_back(
            irot::rotate_y(i),
            glm::angleAxis(glm::radians(90.*i), dvec3(0, 1, 0)));
        r.emplace_back(
            irot::rotate_z(i),
            glm::angleAxis(glm::radians(90.*i), dvec3(0, 0, 1)));
        r.emplace_back(
            irot::rotate_xyz(i),
            glm::angleAxis(glm::radians(120.*i),
                           glm::normalize(dvec3(1, 1, 1))));
    }

    r_copy = r;
    for (auto u: r_copy)
        for (auto v: r_copy)
            r.emplace_back(u.first * v.first, u.second * v.second);

    r_copy = r;
    for (auto u: r_copy)
        r.emplace_back(~u.first, glm::inverse(u.second));

    return r;
}();

static const auto points = []{
    list<ivec3> p;
    for (int i = 0; i < 100; i++) {
        auto get = [] { return rand() % 101 - 50; };
        p.emplace_back(get(), get(), get());
    }
    return p;
}();

BOOST_AUTO_TEST_CASE(irot_matches_dquat_test) {
    for (auto r: rotations)
        BOOST_CHECK_EQUAL(r.first, irot_from_quat(r.second));
}

BOOST_AUTO_TEST_CASE(irot_equiv_dquat_test) {
    for (auto r: rotations)
        for (auto p: points)
            BOOST_CHECK_EQUAL(r.first * p, int_dvec3(r.second * dvec3(p)));
}

BOOST_AUTO_TEST_CASE(irot_quat_cast_test) {
    for (auto r: rotations) {
        CHECK_EQ(r.first.quat_cast(), r.second);
    }
}

BOOST_AUTO_TEST_CASE(irot_boct_test) {
    for (auto r: rotations)
        for (const auto b: boct::all()) {
            boct c{0};
            for (auto i: ioct::all())
                c[glm::greaterThan(r.first*(ivec3(i.bvec3_cast())*2-1),
                                   ivec3(0))] = b[i];
            BOOST_CHECK_EQUAL(r.first * b, c);
        }
}

BOOST_AUTO_TEST_CASE(iloc_test) {
    BOOST_CHECK_EQUAL(iloc() * ivec3(5, 6, 7), ivec3(5, 6, 7));
    BOOST_CHECK_EQUAL((iloc{ivec3(5, 0, 0), {}} * ivec3(5, 6, 7)),
                      ivec3(10, 6, 7));
    BOOST_CHECK_EQUAL((iloc{ivec3(5, 0, 0), irot::rotate_z()} * ivec3(5, 6, 7)),
                      ivec3(-1, 5, 7));
    iloc l{ivec3(5, 0, 0), irot::rotate_z()};
    BOOST_CHECK_EQUAL((~l * l), iloc{});
}
