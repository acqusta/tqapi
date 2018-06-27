#ifndef _MYUTILS_SHMEM_QUEUE_H
#define _MYUTILS_SHMEM_QUEUE_H

#include <iostream>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <string>
#include <memory>

namespace myutils {

    using namespace std;
    using namespace std::chrono;

    class ShmemQueue {
        atomic<int32_t>  m_rw_mtx;
        volatile int32_t m_data_realsize;
        volatile int32_t m_data_size;
        volatile int32_t m_read_pos;
        volatile int32_t m_write_pos;
        char             m_data[1];

    public:
        void init(int32_t size) {
            m_rw_mtx = 0;
            int32_t data_size = size - (size_t)&(((ShmemQueue*)0)->m_data);
            m_data_size = m_data_realsize = data_size;
            m_read_pos = m_write_pos = 0;
            memset(m_data, 0, data_size);
        }

        inline bool push(const char* data, size_t size);
        inline bool poll(const char** data, size_t* size);
        inline bool pop();
    };

    bool ShmemQueue::push(const char* data, size_t size)
    {
        bool ret = false;
        auto begin_time = system_clock::now();
        do {
            int32_t v = 0;
            if (m_rw_mtx.compare_exchange_strong(v, 1)) {
                if (m_read_pos <= m_write_pos) {
                    // ---R----W---
                    if (m_data_size - m_write_pos >= (int32_t)size + 4) {
                        // after ---R-------W-
                        int32_t len = (int32_t)size;
                        char* p = m_data + m_write_pos;
                        memcpy(p, (char*)&len, 4);
                        memcpy(p + 4, data, size);
                        m_write_pos += (int32_t)size + 4;
                        ret = true;
                    }
                    else if (m_read_pos > (int32_t)size + 4) {
                        // after  ---W---R-------
                        m_data_size = m_write_pos;
                        int32_t len = (int32_t)size;
                        char* p = m_data;
                        memcpy(p, (char*)&len, 4);
                        memcpy(p + 4, data, size);
                        m_write_pos = (int32_t)size + 4;
                        ret = true;
                    }
                    else {
                        //assert(false);
                        ret = false;
                        //return false;
                    }
                }
                else if (m_read_pos - m_write_pos > (int32_t)size + 4) {
                    //       ---W-------R----
                    // after -------W---R----
                    int32_t len = (int32_t)size;
                    char* p = m_data + m_write_pos;
                    memcpy(p, (char*)&len, 4);
                    memcpy(p + 4, data, size);
                    m_write_pos += (int32_t)size + 4;
                    ret = true;
                }
                m_rw_mtx--;
                return ret;
            }
        } while (system_clock::now() - begin_time < milliseconds(10));

        return false;
    }

    bool ShmemQueue::poll(const char** data, size_t* size)
    {
        bool ret = false;
        auto begin_time = system_clock::now();
        do {
            int32_t v = 0;
            if (m_rw_mtx.compare_exchange_strong(v, 1)) {
                if (m_read_pos < m_write_pos) {
                    char* p = m_data + m_read_pos;
                    int32_t pkt_size = *(int32_t*)p;
                    *data = p + 4;
                    *size = pkt_size;
                    //assert(pkt_size + 4 <= m_write_pos - m_read_pos);
                    if (pkt_size + 4 > m_write_pos - m_read_pos) {
                        cout << pkt_size << "," << m_write_pos << "," << m_read_pos << endl;
                        assert(0);
                    }
                    ret = true;
                }
                else if (m_read_pos > m_write_pos) {
                    char* p = m_data + m_read_pos;
                    int32_t pkt_size = *(int32_t*)p;
                    *data = p + 4;
                    *size = pkt_size;
                    //assert( pkt_size + 4 <= m_data_size - m_read_pos);
                    if ( pkt_size + 4 > m_data_size - m_read_pos) {
                        cout << *size << "," << m_data_realsize << "," << m_read_pos;
                        assert(false);
                    }
                    ret = true;
                }
                m_rw_mtx--;
                return ret;
            }

        } while (system_clock::now() - begin_time < milliseconds(10));

        return false;
    }

    bool ShmemQueue::pop()
    {
        bool ret = false;
        auto begin_time = system_clock::now();
        do {
            int32_t v = 0;
            if (m_rw_mtx.compare_exchange_strong(v, 1)) {
                char* p = m_data + m_read_pos;
                int32_t pkt_size = *(int32_t*)p;

                if (m_read_pos < m_write_pos) {
                    m_read_pos += pkt_size + 4;
                    assert(m_read_pos <= m_write_pos);
                    if (m_read_pos == m_write_pos) {
                        m_read_pos = m_write_pos = 0;
                        m_data_size = m_data_realsize; // Just ensure
                    }
                }
                else if (m_read_pos > m_write_pos) {
                    m_read_pos += pkt_size + 4;
                    assert(m_read_pos <= m_data_size);
                    if (m_read_pos == m_data_size) {
                        m_read_pos = 0;
                        m_data_size = m_data_realsize;
                    }
                }

                m_rw_mtx--;
                return ret;
            }
        } while (system_clock::now() - begin_time < milliseconds(10));

        return false;
    }


}

#endif
