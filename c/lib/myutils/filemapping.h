#ifndef _MYUTILS_FILEMAPPING_H
#define _MYUTILS_FILEMAPPING_H

#include <stdint.h>
#include <string>

#if _WIN32
#  include <WinSock2.h>
#  include <Windows.h>
#  include <tchar.h>
#endif

namespace myutils {

    using namespace std;


#ifdef _WIN32
    class FileMapping {
    public:

        FileMapping() :
            m_hFile(INVALID_HANDLE_VALUE),
            m_hMapFile(NULL),
            m_pMapAddress(NULL),
            m_filesize(0)
        {
        }

        ~FileMapping() {
            close();
        }

        bool create_file(const string& filename, uint32_t filesize);

        bool open_file(const string& filename, bool read_only);

        bool create_shmem(const string& name, uint32_t filesize);

        bool open_shmem(const string& name, bool read_only);

        void close() {
            if (m_pMapAddress) {
                UnmapViewOfFile(m_pMapAddress);
                m_pMapAddress = nullptr;
            }
            if (m_hMapFile) {
                CloseHandle(m_hMapFile);
                m_hMapFile = nullptr;
            }
            if (m_hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
            }
        }

        bool is_attached() {
            return m_pMapAddress != nullptr;
        }

        inline char* addr() {
            return m_pMapAddress;
        }

        inline int size() {
            return m_filesize;
        }

        string id() { return m_id; }

    public:
        int     m_filesize;
        HANDLE  m_hFile;
        HANDLE  m_hMapFile;
        char*   m_pMapAddress;
        string  m_id;
    };

#else

    class FileMapping {
    public:

        FileMapping() :
            m_pMapAddress(nullptr),
            m_filesize(0),
            m_fd(-1)
        {
        }

        ~FileMapping() {
            close();
        }

        bool create_file  (const string& filename, uint32_t filesize);
        bool create_shmem (const string& name, uint32_t filesize);
        bool open_file    (const string& filename, bool read_only);
        bool open_shmem   (const string& name, bool read_only);

        void close();

        bool is_attached() {
            return m_pMapAddress != nullptr;
        }

        inline char* addr() {
            return m_pMapAddress;
        }

        inline int size() {
            return m_filesize;
        }

        string id() { return m_id; }

    private:        
        char*       m_pMapAddress;
        uint32_t    m_filesize;
        int         m_fd;
        string      m_id;

    };

#endif

}

#endif
