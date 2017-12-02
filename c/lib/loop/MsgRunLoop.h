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
            close();
        }

        MessageLoop& msg_loop() { return m_msg_loop; }
    protected:
        void join() {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
        void close() {
            if (m_thread) {
                m_msg_loop.PostTask(bind(&loop::MessageLoop::QuitNow, &m_msg_loop));
                if (m_thread->joinable())
                    m_thread->join();
                delete m_thread;
                m_thread = nullptr;
            }
        }
        MessageLoop m_msg_loop;

    private:

        void run() {
            RunLoop run(&m_msg_loop);
            run.Run();
        }

        std::thread*  m_thread;
    };

}
#endif
