#ifndef PGAMECC_ENTROPY_H
#define PGAMECC_ENTROPY_H

#include <pgamecc/types.h>

#include <memory>

namespace pgamecc {

namespace entropy {

bool coin();
int dice(int max);
double uniform();
double normal();
double exp(double lambda = 1);
double trunc_exp(double lambda, double cutoff);
int poisson(double mean);

}

class PerlinNoise {
    struct impl;
    std::unique_ptr<impl> p;

public:
    PerlinNoise();
    ~PerlinNoise();
    void set_frequency(double);
    void set_lacunarity(double);
    void set_persistence(double);
    void set_octaves(int);
    void reseed();

    double operator()(dvec2) const;
    double operator()(dvec3) const;
};

class RidgedNoise {
    struct impl;
    std::unique_ptr<impl> p;

public:
    RidgedNoise();
    ~RidgedNoise();
    void set_frequency(double);
    void set_lacunarity(double);
    void set_octaves(int);
    void reseed();

    double operator()(dvec2) const;
    double operator()(dvec3) const;
};

}

#endif
