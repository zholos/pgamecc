#include "entropy.h"

#include <random>
#include <stdexcept>

#include NOISE_INCLUDE_FILE

using std::mt19937;
using std::random_device;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::normal_distribution;
using std::exponential_distribution;
using std::poisson_distribution;
using std::domain_error;

using namespace pgamecc;

namespace {
// TODO: put some preconstructed distributions in class together with generator
thread_local mt19937 gen{random_device()()};
}

bool
entropy::coin() {
    return uniform_int_distribution<int>(0, 1)(gen);
}

int
entropy::dice(int max) {
    if (max <= 0)
        throw domain_error("dice(max): max must be positive");
    return uniform_int_distribution<int>(0, max-1)(gen);
}

double
entropy::uniform() {
    return uniform_real_distribution<double>(0, 1)(gen);
}

double
entropy::normal() {
    return normal_distribution<double>(0, 1)(gen);
}

double
entropy::exp(double lambda) {
    return exponential_distribution<double>(lambda)(gen);
}

double
entropy::trunc_exp(double lambda, double cutoff) {
    if (cutoff <= 0)
        throw domain_error("trunc_exp(): cutoff must be positive");
    auto c = std::exp(cutoff * -lambda);
    return std::log(c + (1-c)*uniform())/-lambda;
}

int
entropy::poisson(double mean) {
    return poisson_distribution<int>(mean)(gen);
}


struct PerlinNoise::impl : noise::module::Perlin {};

PerlinNoise::PerlinNoise() : p(new impl) {}
PerlinNoise::~PerlinNoise() = default;

void PerlinNoise::set_frequency(double v) { p->SetFrequency(v); }
void PerlinNoise::set_lacunarity(double v) { p->SetLacunarity(v); }
void PerlinNoise::set_persistence(double v) { p->SetPersistence(v); }
void PerlinNoise::set_octaves(int v) { p->SetOctaveCount(v); }

void
PerlinNoise::reseed()
{
    p->SetSeed(gen());
}

double
PerlinNoise::operator()(dvec2 v) const
{
    // might as well sample at z=.5, since it will interpolate 8 corners anyway
    return p->GetValue(v.x, v.y, .5);
}

double
PerlinNoise::operator()(dvec3 v) const
{
    return p->GetValue(v.x, v.y, v.z);
}


struct RidgedNoise::impl : noise::module::RidgedMulti {};

RidgedNoise::RidgedNoise() : p(new impl) {}
RidgedNoise::~RidgedNoise() = default;

void RidgedNoise::set_frequency(double v) { p->SetFrequency(v); }
void RidgedNoise::set_lacunarity(double v) { p->SetLacunarity(v); }
void RidgedNoise::set_octaves(int v) { p->SetOctaveCount(v); }

void
RidgedNoise::reseed()
{
    p->SetSeed(gen());
}

double
RidgedNoise::operator()(dvec2 v) const
{
    // might as well sample at z=.5, since it will interpolate 8 corners anyway
    return p->GetValue(v.x, v.y, .5);
}

double
RidgedNoise::operator()(dvec3 v) const
{
    return p->GetValue(v.x, v.y, v.z);
}
