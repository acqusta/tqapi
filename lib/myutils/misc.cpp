#include <chrono>
#include <random>
#include <limits.h>
#include <stdint.h>

#include "misc.h"


namespace myutils{

    using namespace std;
    using namespace chrono;

    int random()
    {
        static std::default_random_engine         g_generator;
        static std::uniform_int_distribution<int> g_distribution(1, INT32_MAX);
        static bool g_inited = false;

        if (!g_inited) {
            g_inited = true;
            g_generator.seed(system_clock::now().time_since_epoch().count());
        }
        return g_distribution(g_generator);
    }


}
