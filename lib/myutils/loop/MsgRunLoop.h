#ifndef _LOOP_MSGRUNLOOP_H
#define _LOOP_MSGRUNLOOP_H

#include <thread>
#include <functional>

#include "MessageLoop.h"
#include "RunLoop.h"

namespace loop {

    using namespace std;

    class MsgLoopRun{
    public:

        MsgLoopRun() {
            m_thread = new std::thread(bind(&MsgLoopRun::run, this));
        }

        virtual ~MsgLoopRun() {
            close_loop();
        }

        MessageLoop& msg_loop() { return m_msg_loop; }

        void close_loop() {
            if (m_thread) {
                m_msg_loop.post_task(bind(&loop::MessageLoop::quit_now, &m_msg_loop));
                if (m_thread->joinable())
                    m_thread->join();
                delete m_thread;
                m_thread = nullptr;
            }
        }

        void  post_task(std::function<void()> func, int timeout = 0)
        {
            m_msg_loop.post_delayed_task(func, timeout);
        }

    protected:
        void join() {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
        MessageLoop m_msg_loop;

    private:

        void run() {
            RunLoop run_loop(&m_msg_loop);
            run_loop.run();
        }

        std::thread*  m_thread;
    };

}
#endif
