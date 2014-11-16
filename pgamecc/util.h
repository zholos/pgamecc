#ifndef PGAMECC_UTIL_H
#define PGAMECC_UTIL_H

#include <iostream>

namespace pgamecc {

class Timer {
    long long from;

    public:
    Timer();
    long long elapsed_us();
    long long elapsed_ms() { return elapsed_us() / 1000; }
};

}

#endif
