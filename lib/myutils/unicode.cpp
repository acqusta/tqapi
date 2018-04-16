#include <assert.h>
#include "unicode.h"

using namespace std;

#ifdef _WIN32
#include <Windows.h>
#include "unicode.h"

int GBKToUTF8(const unsigned char * lpGBKStr, unsigned char * lpUTF8Str, int nUTF8StrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;

    if (!lpGBKStr)
        return 0;

    nRetLen = ::MultiByteToWideChar(CP_ACP, 0, (char *)lpGBKStr, -1, NULL, NULL);
    lpUnicodeStr = new WCHAR[nRetLen + 1]; 
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

    return nRetLen;
}

int UTF8ToGBK(const unsigned char * lpUTF8Str, unsigned char * lpGBKStr, int nGBKStrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;

    if (!lpUTF8Str)
        return 0;

    nRetLen = ::MultiByteToWideChar(CP_UTF8, 0, (char *)lpUTF8Str, -1, NULL, NULL);
    lpUnicodeStr = new WCHAR[nRetLen + 1];
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

    return nRetLen;
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
        return nRetLen * 2;
    }
}
string gbk_to_utf8(const string& gbk)
{
    int buf_size = (int)gbk.size() * 2 + 10;
    char* buf = new char[buf_size]; memset(buf, 0, buf_size);
    int len = GBKToUTF8((const unsigned  char*)gbk.c_str(), (unsigned char*)buf, buf_size);
    string utf8(buf, strlen(buf));
    delete[] buf;
    return utf8;
}

string utf8_to_gbk(const string& utf8)
{
    int buf_size = (int)utf8.size() * 4 + 10;
    char* buf = new char[buf_size]; memset(buf, 0, buf_size);
    int len = UTF8ToGBK((const unsigned  char*)utf8.c_str(), (unsigned char*)buf, buf_size);
    string gbk(buf, strlen(buf));
    delete[] buf;
    return gbk;
}
std::string utf8_to_local(const std::string& utf8)
{
    return utf8_to_gbk(utf8);
}

std::string gbk_to_local(const std::string& gbk)
{
    return gbk;
}

std::string local_to_utf8(const string& str)
{
    return gbk_to_utf8(str);
}

std::string utf8_to_utf16(const std::string& utf8)
{
    int buf_size = (int)utf8.size() * 4 + 10;
    char* buf = new char[buf_size];
    memset(buf, 0, buf_size);

    int len = UTF8ToUTF16((const unsigned  char*)utf8.c_str(), (unsigned char*)buf, buf_size);
    string gbk(buf, len);
    delete[] buf;
    return gbk;
}


#else

#include <iconv.h>

static string convert(const char* from, const char* to, const char* src, size_t size)
{
    if (size == 0 ) return "";

    size_t buf_len = size*2 + 10;
    char* buf = new char[buf_len];

    const char* src_orig = src;
    char* dst = buf;
    size_t dst_len = buf_len;
    size_t src_len = size;

    iconv_t ic = iconv_open(to, from);
    CHECK( (size_t)ic != -1 ) << "iconv_open failed";

    size_t ret = iconv(ic, (char**)&src, &src_len, &dst, &dst_len);

    if (ret == (size_t)-1){
        LOG(ERROR) << "iconv failed: " << from << "," << to << ":" << strerror(errno);
        delete[] buf;
        iconv_close(ic);
        return src_orig;
    }

    string str(buf, buf_len - dst_len);
    delete[] buf;
    iconv_close(ic);
    return str;
}

string gbk_to_utf8(const string& gbk)
{
    return convert("GBK", "UTF-8", gbk.c_str(), gbk.size());
}

string utf8_to_gbk(const string& utf8)
{
    return convert("UTF-8", "GBK", utf8.c_str(), utf8.size());
}

std::string utf8_to_local(const std::string& utf8)
{
    return utf8;
}

std::string gbk_to_local(const std::string& gbk)
{
    return convert("GBK", "UTF-8", gbk.c_str(), gbk.size());
}

std::string local_to_utf8(const string& str)
{
    return str;
}


#endif
