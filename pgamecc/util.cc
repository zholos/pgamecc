#include "util.h"

#include <chrono>
#include <ratio>

using namespace pgamecc;


namespace {
typedef std::chrono::steady_clock clock_type;
}

static auto start_time = clock_type::now();

static long long
elapsed() {
    return std::chrono::duration_cast<
        std::chrono::duration<long long, std::micro>>(
        clock_type::now() - start_time).count();
}

Timer::Timer() :
    from(elapsed())
{
}

long long
Timer::elapsed_us()
{
    return elapsed() - from;
}
