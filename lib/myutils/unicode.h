#ifndef _UNICODE_H
#define _UNICODE_H

#include <string>

std::string gbk_to_utf8    (const std::string& gbk);
std::string utf8_to_gbk    (const std::string& utf8);
std::string utf8_to_local  (const std::string& utf8);
std::string gbk_to_local   (const std::string& gbk);
std::string local_to_utf8  (const std::string& str);
std::string utf8_to_utf16  (const std::string& utf8);

std::string& gbk_to_utf8   (const std::string& gbk , std::string* out);
std::string& utf8_to_gbk   (const std::string& utf8, std::string* out);
std::string& utf8_to_local (const std::string& utf8, std::string* out);
std::string& gbk_to_local  (const std::string& gbk , std::string* out);
std::string& local_to_utf8 (const std::string& str , std::string* out);
std::string& utf8_to_utf16 (const std::string& utf8, std::string* out);

#endif
