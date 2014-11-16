#define BOOST_TEST_MODULE entropy
#include <boost/test/included/unit_test.hpp>

#include "entropy.h"

#include <iterator>
#include <set>
#include <stdexcept>

using std::set;
using std::inserter;
using std::generate_n;
using std::domain_error;

using namespace pgamecc;
using namespace pgamecc::entropy;


//
// The tests
//

int n = 1000;

template<typename Func>
auto random_set(const Func& f) {
    set<decltype(f())> result;
    generate_n(inserter(result, result.end()), n, f);
    return result;
}

template<typename T, typename U>
bool in_range(T x, U a, U b) {
    return a <= *x.begin() && *x.rbegin() < b;
}

template<typename T>
bool diverse(T x) {
    return x.size() > 1;
}

BOOST_AUTO_TEST_CASE(entropy_coin) {
    BOOST_CHECK_THROW(dice(-1), domain_error);
    BOOST_CHECK_THROW(dice(0),  domain_error);

    auto coin_rolls = random_set(coin);
    BOOST_CHECK(in_range(coin_rolls, 0, 2));
    BOOST_CHECK(diverse(coin_rolls));

    for (int j: { 1, 2, 3, 4, 5, 6, 20, 100, 1000, 1000000 }) {
        auto dice_rolls = random_set([&] { return dice(j); });
        BOOST_CHECK(in_range(dice_rolls, 0, j));
        if (j > 1)
            BOOST_CHECK(diverse(dice_rolls));
    }

    auto uniform_rolls = random_set(uniform);
    BOOST_CHECK(in_range(uniform_rolls, 0, 1));
    BOOST_CHECK(diverse(uniform_rolls));

    auto normal_rolls = random_set(normal);
    BOOST_CHECK(diverse(normal_rolls));

    auto poisson_rolls = random_set([] { return poisson(2.5); });
    BOOST_CHECK(*poisson_rolls.begin() >= 0);
    BOOST_CHECK(diverse(poisson_rolls));
}


BOOST_AUTO_TEST_CASE(entropy_noise) {
    PerlinNoise noise;

    set<double> perlin_samples;
    for (int y = -10; y <= 10; y++)
        for (int x = -10; x <= 10; x++)
            perlin_samples.insert(noise(dvec2{x, y}));

    // "The noise module outputs [...] usually range from -1.0 to +1.0, but
    // there are no guarantees that all output values will exist within that
    // range." - libnoise docs
    BOOST_CHECK(in_range(perlin_samples, -2, 2));

    BOOST_CHECK(diverse(perlin_samples));
}
