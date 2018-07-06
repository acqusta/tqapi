#ifndef _LOOP_MSGLOOPFUTURE_H
#define _LOOP_MSGLOOPFUTURE_H

#include <atomic>
#include <memory>
#include <string>
#include "MessageLoop.h"

namespace loop {

    using namespace std;

    template <typename T_VALUE, typename T_ERROR> class Promise;
    template <typename T_VALUE, typename T_ERROR> class Future;
    template <typename T_VALUE, typename T_ERROR> class FutureLoop;

    template <typename T_VALUE, typename T_ERROR>
    class FutureLoop {
    public:
        FutureLoop(loop::MessageLoop* loop, Promise<T_VALUE, T_ERROR>* prom)
            : m_loop(loop)
            , m_prom(prom)
        {
            m_prom->lock();
        }

        FutureLoop(const FutureLoop& r){
            m_loop = r.m_loop;
            m_prom = r.m_prom;
            m_prom->lock();
        }

        ~FutureLoop() {
            m_prom->unlock();
        }

        FutureLoop<T_VALUE, T_ERROR> on_success(std::function<void(shared_ptr<T_VALUE>)> func) {
            m_prom->m_success.first = m_loop;
            m_prom->m_success.second = func;
            
            return *this;
        }

        FutureLoop<T_VALUE, T_ERROR> on_failure(function<void(shared_ptr<T_ERROR>)> func) {
            m_prom->m_failure.first = m_loop;
            m_prom->m_failure.second = func;

            return *this;
        }
    private:
        loop::MessageLoop* m_loop;
        Promise<T_VALUE, T_ERROR>*        m_prom;
    };

    template<typename T_VALUE, typename T_ERROR>
    class Future {
    public:
        Future(Promise<T_VALUE, T_ERROR>* prom) : m_prom(prom) {
            //m_prom->lock();
        }

        ~Future() {
            m_prom->unlock();
        }

        FutureLoop<T_VALUE, T_ERROR> in_loop(loop::MessageLoop* loop) {
            return FutureLoop<T_VALUE, T_ERROR>(loop, m_prom);
        }

    private:
        Promise<T_VALUE, T_ERROR>* m_prom;
    };

    template<typename T_VALUE, typename T_ERROR>
    class Promise {
        friend FutureLoop<T_VALUE, T_ERROR>;
    public:
        Promise(loop::MessageLoop* msgloop)
            : m_lock(1)
            , m_msgloop(msgloop)
        {
        }

        Future<T_VALUE, T_ERROR> get_future() {
            if (m_lock == 1)
                return Future<T_VALUE, T_ERROR>(this);
            else
                throw std::runtime_error("get_future can't be call twice");
        }

        void lock() {
            m_lock++;
        }

        void unlock() {
            m_lock--;
        }

        void set_value(shared_ptr<T_VALUE> value) {
            while (m_lock != 0) {}
            if (m_success.second) {
                auto msgloop = m_success.first ? m_success.first : m_msgloop;
                msgloop->post_task(std::bind(m_success.second, value));
            }
        }

        void set_error(shared_ptr<T_ERROR> error) {
            while (m_lock != 0) {}
            if (m_failure.second) {
                auto msgloop = m_failure.first ? m_failure.first : m_msgloop;
                msgloop->post_task(std::bind(m_failure.second, error));
            }
        }

    private:
        atomic<int> m_lock;
        loop::MessageLoop*  m_msgloop;
        std::pair<loop::MessageLoop*, function<void(shared_ptr<T_VALUE>)>>  m_success;
        std::pair<loop::MessageLoop*, function<void(shared_ptr<T_ERROR>)>>  m_failure;
    };

}

#endif

