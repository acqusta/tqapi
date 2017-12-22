#ifdef _WIN32

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "filemapping.h"

using namespace std;
using namespace myutils;

string ConvertErrorCodeToString(DWORD ErrorCode)
{
    HLOCAL LocalAddress = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, ErrorCode, 0, (char*)&LocalAddress, 0, NULL);

    string ret((char*)LocalAddress);
    LocalFree(LocalAddress);
    return ret;
}

// TODO:
//   check if file exist, and  filesize is correct
bool FileMapping::create_file(const string& filename,  uint32_t filesize)
{
    close();

    m_id = filename;
    m_filesize = filesize;

    m_hFile = CreateFile(filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (m_hFile == INVALID_HANDLE_VALUE) {
        //std::cout << "Failed to create " << filename << "," << filename << ": " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);
    DWORD dwSysGran = SysInfo.dwAllocationGranularity;
    filesize = ((filesize + dwSysGran - 1) / dwSysGran) * dwSysGran;

    m_hMapFile = CreateFileMapping(m_hFile,         // current file handle
        NULL, //&secu_attr,                         // default security
        PAGE_READWRITE,                             // read/write permission
        0,
        filesize & 0xFFFFFFFF,                      // size of mapping object, low
         NULL);                                     // name of mapping object

    if (m_hMapFile == NULL) {
        //LOG(ERROR) << "CreateFileMapping failed " << " " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    // Map the view and test the results.
    m_pMapAddress = (char*)MapViewOfFile(m_hMapFile,
        FILE_MAP_ALL_ACCESS, // read/write
        0,
        0,
        filesize);

    if (m_pMapAddress == NULL) {
        //LOG(ERROR) << "MapViewOfFile failed: " << " " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    return true;
}

bool FileMapping::create_shmem(const string& name, uint32_t filesize)
{
    close();

    m_id = name;
    m_filesize = filesize;

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);
    DWORD dwSysGran = SysInfo.dwAllocationGranularity;
    filesize = ((filesize + dwSysGran - 1) / dwSysGran) * dwSysGran;

    m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,         // current file handle
        NULL, //&secu_attr,                                       // default security
        PAGE_READWRITE,                             // read/write permission
        0,
        filesize & 0xFFFFFFFF,                      // size of mapping object, low
        name.c_str());        // name of mapping object

    if (m_hMapFile == NULL) {
        //LOG(ERROR) << "CreateFileMapping failed " << " " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    // Map the view and test the results.
    m_pMapAddress = (char*)MapViewOfFile(m_hMapFile,
        FILE_MAP_ALL_ACCESS, // read/write
        0,
        0,
        filesize);

    if (m_pMapAddress == NULL) {
        //LOG(ERROR) << "MapViewOfFile failed: " << " " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    return true;
}

bool FileMapping::open_file(const string& filename, bool read_only)
{
    close();

    m_id = filename;

    m_hFile = CreateFile(filename.empty() ? NULL : filename.c_str(),
        read_only ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
        FILE_SHARE_READ | FILE_SHARE_WRITE, //read_only ? 0 : FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (m_hFile == INVALID_HANDLE_VALUE) {
        //VLOG(1) << "Failed to open " << str << " " << ConvertErrorCodeToString(GetLastError());
        return false;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(m_hFile, &size)) {
        //LOG(ERROR) << "Can't get filesize ";
        return false;
    }
    if (size.QuadPart > std::numeric_limits<uint32_t>::max()) {
        //LOG(ERROR) << "FileSize is over 4G";
        return false;
    }

    m_filesize = size.LowPart;

    m_hMapFile = CreateFileMapping(m_hFile,              // current file handle
        NULL,               // default security
        read_only ? PAGE_READONLY : PAGE_READWRITE,     // read/write permission
        0,
        m_filesize,
        NULL);              // name of mapping object

    if (m_hMapFile == NULL) {
        //LOG(ERROR) << "CreateFileMapping failed " << GetLastError();
        return false;
    }

    // Map the view and test the results.
    m_pMapAddress = (char*)MapViewOfFile(m_hMapFile,
        read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, // read/write
        0,
        0,
        m_filesize);

    if (m_pMapAddress == NULL) {
        //LOG(ERROR) << "MapViewOfFile failed: " << GetLastError();
        return false;
    }

    return true;
}


bool FileMapping::open_shmem(const string& name, uint32_t filesize, bool read_only)
{
    close();

    m_id = name;
    m_filesize = filesize;

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);
    DWORD dwSysGran = SysInfo.dwAllocationGranularity;
    filesize = ((filesize + dwSysGran - 1) / dwSysGran) * dwSysGran;

    m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
        NULL,
        name.c_str());

    if (m_hMapFile == NULL) {
        //std::cout << "OpenFileMapping failed: " << ConvertErrorCodeToString(GetLastError());
        return false;
    }
    // Map the view and test the results.
    m_pMapAddress = (char*)MapViewOfFile(m_hMapFile,
        read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, // read/write
        0,
        0,
        filesize);

    if (m_pMapAddress == NULL) {
        //LOG(ERROR) << "MapViewOfFile failed: " << GetLastError();
        return false;
    }

    return true;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <string>
#include <assert.h>
#include <iostream>
//#include "glog/logging.h"

#include "filemapping.h"

using namespace std;
using namespace myutils;

bool FileMapping::create_file(const string& filename, uint32_t filesize)
{
    m_id = filename;
    int fd = ::open(filename.c_str(), O_CREAT | O_RDWR, S_IRWXU);
    if ( fd == -1) {
        //VLOG(1) <<"Can't create " << filename <<", error=" << strerror(errno);
        return false;
    }

    fchmod(fd, S_IRUSR | S_IWUSR);
    
    char* buf = new char[filesize];
    write(fd, buf, filesize);
    delete[] buf;

    char* p = (char*)mmap(nullptr, filesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if ( p == (char*)-1) {
        ::close(fd);
        //LOG(FATAL) <<"mmap failed: " << filename <<", error=" << strerror(errno);
        return false;
    }

    m_pMapAddress = p;
    m_fd = fd;
    m_filesize = filesize;

    return true;
}

bool FileMapping::open_file(const string& filename, bool read_only)
{
    m_id = filename;
    struct stat st;
    if (stat(filename.c_str(), &st)) {
        // LOG(ERROR) <<"Can't stat " << filename <<", error=" << strerror(errno);
        return false;     
    }
    if (st.st_size == 0) {
        // LOG(ERROR) <<"Wrong file size " << filename <<", size=" << st.st_size;
        return false;
    }

    int flag = read_only? O_RDONLY : O_RDWR;
    int fd = ::open(filename.c_str(), flag);
    if ( fd == -1) {
        // LOG(ERROR) <<"Can't open " << filename <<", error=" << strerror(errno);
        return false;
    }

    //LOG(INFO) << "open filemapping " << filename << ", " << st.st_size
    //          << " " << (read_only?"READONLY" : "WRITABLE");

    int prot = read_only ? PROT_READ : (PROT_READ|PROT_WRITE);

    m_pMapAddress = (char*)mmap(nullptr, st.st_size, prot, MAP_SHARED, fd, 0);
    if (!m_pMapAddress) {
        // LOG(ERROR) <<"mmap failed: " << filename <<", error=" << strerror(errno);
        ::close(fd);
        return false;
    }

    //CHECK(st.st_size < UINT32_MAX) << "Only support 4G memory file";
    assert(st.st_size < UINT32_MAX);// << "Only support 4G memory file";
    m_fd = fd;
    m_filesize = static_cast<uint32_t>(st.st_size);

    return true;
}

void FileMapping::close()
{
    if (m_pMapAddress) {
        munmap(m_pMapAddress, m_filesize);
        m_pMapAddress = nullptr;
    }
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
    }
}

#endif
