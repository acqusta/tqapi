#include <assert.h>
#include <iostream>
#include <string.h>
#include "unicode.h"

//using namespace std;

#ifdef _WIN32
#include <Windows.h>

int GBKToUTF8(const unsigned char * lpGBKStr, unsigned char * lpUTF8Str, int nUTF8StrLen)
{
    if (!lpGBKStr)
        return 0;

    int nRetLen = ::MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, NULL, NULL);
    wchar_t* lpUnicodeStr = new WCHAR[nRetLen]; 
    nRetLen = ::MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, lpUnicodeStr, nRetLen);
    if (!nRetLen)
        return 0;

    nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, NULL, 0, NULL, NULL);

    if (!lpUTF8Str)
    {
        if (lpUnicodeStr)
            delete[]lpUnicodeStr;
        return nRetLen;
    }

    if (nUTF8StrLen < nRetLen)
    {
        if (lpUnicodeStr)
            delete[]lpUnicodeStr;
        return 0;
    }

    nRetLen = ::WideCharToMultiByte(CP_UTF8, 0, lpUnicodeStr, -1, (char *)lpUTF8Str, nUTF8StrLen, NULL, NULL);

    if (lpUnicodeStr)
        delete[]lpUnicodeStr;

    return nRetLen > 0 ? nRetLen - 1 : nRetLen;
}

int UTF8ToGBK(const unsigned char * lpUTF8Str, unsigned char * lpGBKStr, int nGBKStrLen)
{
    if (!lpUTF8Str)
        return 0;

    int nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL);
    wchar_t* lpUnicodeStr = new WCHAR[nRetLen];
    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, lpUnicodeStr, nRetLen);
    if (!nRetLen)
        return 0;

    nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, NULL, NULL, NULL, NULL);

    if (!lpGBKStr)
    {
        if (lpUnicodeStr)
            delete[]lpUnicodeStr;
        return nRetLen;
    }

    if (nGBKStrLen < nRetLen)
    {
        if (lpUnicodeStr)
            delete[]lpUnicodeStr;
        return 0;
    }

    nRetLen = ::WideCharToMultiByte(CP_ACP, 0, lpUnicodeStr, -1, (char *)lpGBKStr, nRetLen, NULL, NULL);

    if (lpUnicodeStr)
        delete[]lpUnicodeStr;

    // Remove the last null charactor
    return nRetLen > 0 ? nRetLen - 1: 0;
}

int UTF8ToUTF16(const unsigned char * lpUTF8Str, unsigned char * lpUTF16Str, int nUTF16StrLen)
{
    int nRetLen = 0;

    if (!lpUTF8Str)
        return 0;

    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL);
    if (!lpUTF16Str) return nRetLen;


    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, (wchar_t*)lpUTF16Str, nUTF16StrLen);
    if (!nRetLen) {
        return 0;
    }
    else {
        return (nRetLen - 1)* 2 ;
    }
}

std::string& gbk_to_utf8(const std::string& gbk, std::string* out)
{
    int buf_size = (int)gbk.size() * 2 + 10;
    out->resize(buf_size);
    char* buf = (char*)out->data();
    int len = GBKToUTF8((const unsigned  char*)gbk.c_str(), (unsigned char*)buf, buf_size);
    //string utf8(buf, strlen(buf));
    out->resize(len);
    return *out;
}

std::string& utf8_to_gbk(const std::string& utf8, std::string* out)
{
    int buf_size = (int)utf8.size() * 4 + 10;
    out->resize(buf_size);
    char* buf = (char*)out->data();
    int len = UTF8ToGBK((const unsigned  char*)utf8.c_str(), (unsigned char*)buf, buf_size);
    out->resize(len);
    return *out;
}

std::string& utf8_to_local(const std::string& utf8, std::string* out)
{
    return utf8_to_gbk(utf8, out);
}

std::string& gbk_to_local(const std::string& gbk, std::string* out)
{
    *out = gbk;
    return *out;
}

std::string& local_to_utf8(const std::string& str, std::string* out)
{
    return gbk_to_utf8(str, out);
}

std::string& utf8_to_utf16(const std::string& utf8, std::string* out)
{
    int buf_size = (int)utf8.size() * 4 + 10;
    out->resize(buf_size);
    char* buf = (char*)out->data();

    int len = UTF8ToUTF16((const unsigned  char*)utf8.c_str(), (unsigned char*)buf, buf_size);
    out->resize(len);
    return *out;
}


#else

#include <iconv.h>

static std::string& convert(const char* from, const char* to, const char* _src, size_t size, std::string* out)
{
    // std::cout<<"convert " << from <<"," << to << "," << size << std::endl;
    if (size == 0 ) {
        *out = "";
        return *out;
    }

    std::string orig_src(_src, size);

    size_t buf_len = size*2 + 10;
    out->resize(buf_len);

    char* out_buf = (char*)out->data();

    char to_ignore[100];
    // char from_ignore[100];
    sprintf(to_ignore, "%s//IGNORE", to);
    iconv_t ic = iconv_open(to_ignore, from);
    // iconv_t ic = iconv_open(to, from);

    if ((size_t)ic == -1) {
        std::cerr << "iconv_open error: " << from << " -> " << to << std::endl;
        *out = "";
        return *out;
    }


    //while (true) {
    for (int i = 0; i < 100; i++) { // max try 100
        char* src = (char*)orig_src.data();
        char* dst = out_buf;
        size_t dst_len = buf_len;
        size_t src_len = size;

        size_t ret = iconv(ic, (char**)&src, &src_len, &dst, &dst_len);
        if ( (ret == (size_t)-1) && (src - orig_src.data() < size) ) {
            std::cerr << "iconv failed: " << from << "," << to << ":" << strerror(errno)
                << "at " << (src - orig_src.data()) << std::endl;

            if (src - orig_src.data() + 2 < size) {
                if ((uint8_t)src[0] > 0x80 ) {
                    src[0] = ' ';
                    src[1] = ' ';
                }
                else {
                    src[0] = ' ';
                }
            } else {
                src[0] = ' ';
            }
            continue;
        } else {
            iconv_close(ic);
            out->resize(buf_len - dst_len);            
            return *out;
        }
    }
    out->resize(0);
    return *out;
}

std::string& gbk_to_utf8(const std::string& gbk, std::string* out)
{
    return convert("GBK", "UTF-8", gbk.c_str(), gbk.size(), out);
}

std::string& utf8_to_gbk(const std::string& utf8, std::string* out)
{
    return convert("UTF-8", "GBK", utf8.c_str(), utf8.size(), out);
}

std::string& utf8_to_local(const std::string& utf8, std::string* out)
{
    *out = utf8;
    return *out;
}

std::string& local_to_utf8(const std::string& str, std::string* out)
{
    *out = str;
    return *out;
}

std::string& gbk_to_local(const std::string& gbk, std::string* out)
{
    return convert("GBK", "UTF-8", gbk.c_str(), gbk.size(), out);
}

std::string& utf8_to_utf16(const std::string& utf8, std::string* out)
{
    return convert("UTF-8", "UTF-16LE", utf8.c_str(), utf8.size(), out);
}


#endif

std::string gbk_to_utf8(const std::string& gbk)
{
    std::string out;
    gbk_to_utf8(gbk, &out);
    return out;
}

std::string utf8_to_gbk(const std::string& utf8)
{
    std::string out;
    utf8_to_gbk(utf8, &out);
    return out;
}

std::string utf8_to_local(const std::string& utf8)
{
    std::string out;
    utf8_to_local(utf8, &out);
    return out;
}
std::string gbk_to_local(const std::string& gbk)
{
    std::string out;
    gbk_to_local(gbk, &out);
    return out;
}

std::string local_to_utf8(const std::string& str)
{
    std::string out;
    local_to_utf8(str, &out);
    return out;
}

std::string utf8_to_utf16(const std::string& utf8)
{
    std::string out;
    utf8_to_utf16(utf8, &out);
    return out;
}
