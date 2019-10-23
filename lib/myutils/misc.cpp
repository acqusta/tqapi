#include <chrono>
#include <random>
#include <limits.h>
#include <stdint.h>
#ifdef _WIN32
// # include <filesystem>
// namespace fs = std::tr2::sys;
# include <Windows.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif

#include "misc.h"
#include "stringutils.h"

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

    bool parse_addr(const string& addr, string* url, unordered_map<string, string>* properties)
    {
        vector<string> ss;
        split(addr, "?", &ss);
        *url = ss[0];
        if (ss.size() > 1) {
            split(ss[1], "&", &ss);
            for (auto& s : ss) {
                vector<string> tmp;
                split(s, "=", &tmp);
                (*properties)[tmp[0]] = tmp.size() == 2 ? tmp[1] : "";
            }
        }
        return true;
    }

    bool make_abs_dir(const std::string& abs_path)
    {
#ifdef _WIN32
        // if (!fs::exists(abs_path)) {
        //     return fs::create_directories(abs_path);
        // }
        // else {
        //     return true;
        // }
        DWORD attribs = ::GetFileAttributesA(abs_path.c_str());
        if (attribs == INVALID_FILE_ATTRIBUTES) {
            return CreateDirectoryA (abs_path.c_str(), NULL);
        }
        else {
            return (attribs & FILE_ATTRIBUTE_DIRECTORY);
        }
#else
        struct stat s;
        if (stat(abs_path.c_str(), &s) != 0) {
            std::string cmd = "mkdir -p " + string(abs_path);
            return system(cmd.c_str()) == 0;
        }
        else {
            return true;
        }
#endif
    }



}
