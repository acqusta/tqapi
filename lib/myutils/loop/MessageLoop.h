#ifndef _LOOP_MESSAGELOOP_H
#define _LOOP_MESSAGELOOP_H

#include <atomic>
#include <functional>
#include <stdint.h>
#include <list>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "myutils/ipc_common.h"
#include "myutils/concurrentqueue.h"

namespace loop {
    typedef std::function< void(void) > Closure;

    class RunLoop;

    class MessageLoop {
        friend RunLoop;

        struct TaskInfo {
            uint32_t  id;
            Closure   task;
            int64_t   execute_time;

            TaskInfo() { }

            TaskInfo(int a_id, const Closure& a_task, int64_t etime)
                : id(a_id), task((Closure)a_task), execute_time(etime)
            {}
        };

    public:
        MessageLoop();

        ~MessageLoop();

        bool delete_task(uint32_t id);

        uint32_t post_task(const Closure& task);

        uint32_t post_delayed_task(const Closure& task, uint32_t delay); //ms

        // May be only called in the thread of message loop.
        void quit_now();
    private:
        void run(RunLoop* run);

        moodycamel::ConcurrentQueue<uint32_t>  m_deleted_queue;
        moodycamel::ConcurrentQueue<TaskInfo*> m_quque;
        myutils::SharedSemaphore*    m_sem;
        std::atomic<uint32_t>        m_next_id;
        RunLoop*                m_run;


    };

}

#endif