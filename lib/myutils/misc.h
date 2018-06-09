#ifndef _MYUTILS_MISC_H
#define _MYUTILS_MISC_H

#include <unordered_map>
#include <string>

namespace myutils {
    
    int random();

    bool parse_addr(const std::string& addr, std::string* url, std::unordered_map<std::string, std::string>* properties);

}


#endif
