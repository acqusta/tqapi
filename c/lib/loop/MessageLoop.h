#ifndef _LOOP_MESSAGELOOP_H
#define _LOOP_MESSAGELOOP_H

#include <functional>
#include <stdint.h>
#include <list>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace loop {
    typedef std::function< void(void) > Closure;

    class RunLoop;

    class MessageLoop {
        friend RunLoop;
    public:
        MessageLoop();

        ~MessageLoop();

        bool DeleteTask(uint32_t id);

        uint32_t PostTask(const Closure& task);

        uint32_t PostDelayedTask(const Closure& task, uint32_t delay); //ms

        // May be only called in the thread of message loop.
        void QuitNow();
    private:
        void Run(RunLoop* run);

        //apr_pool_t*             m_pool;
        std::mutex                  m_queue_lock;
        std::condition_variable     m_queue_cond;
        uint32_t                m_next_id;
        RunLoop*                m_run;

        struct TaskInfo {
            uint32_t      id;
            Closure       task;
            
            std::chrono::system_clock::time_point  execute_time;


            TaskInfo() { }

            //TaskInfo(const TaskInfo& a) : id(a.id), executeTime(a.executeTime), task(a.task)
            //{}
            TaskInfo(int a_id, const Closure a_task, std::chrono::system_clock::time_point time) :
                id(a_id), task((Closure)a_task), execute_time(time)
            {}
        };

        std::list<TaskInfo>     m_queue;

    };

}

#endif