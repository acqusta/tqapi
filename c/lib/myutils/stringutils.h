#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

static inline void trim(std::string* s)
{
    s->erase(s->begin(), std::find_if(s->begin(), s->end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    s->erase(std::find_if(s->rbegin(), s->rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s->end());
}

static inline std::string trim(const char* s)
{
    std::string tmp(s);
    tmp.erase(tmp.begin(), std::find_if(tmp.begin(), tmp.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    tmp.erase(std::find_if(tmp.rbegin(), tmp.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), tmp.end());
    return tmp;
}

static inline void split(const std::string& s, const std::string& delim, std::vector< std::string >* ret, bool need_trim = true)
{
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos) {
        if (need_trim) { 
            auto tmp = s.substr(last, index - last);
            trim(&tmp);
            ret->push_back(tmp);
        } else {
            ret->push_back(s.substr(last, index - last));
        }
        last = index + 1;
        index = s.find_first_of(delim, last);
    }

    if (index - last>0) {
        if (need_trim) {
            auto tmp = s.substr(last, index - last);
            trim(&tmp);
            ret->push_back(tmp);
        }
        else
            ret->push_back(s.substr(last, index - last));
    }
}

// Move to myutils

static inline std::string hex_str(const std::string& str)
{
    std::stringstream ss;
    ss << std::ios::hex << std::setfill('0') << std::uppercase;
    for (auto it = str.begin(); it != str.end(); it++)
        ss << std::setw(2) << (unsigned int)(unsigned char)*it;

    return ss.str();
}
//

#endif
