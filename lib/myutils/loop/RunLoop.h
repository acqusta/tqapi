#ifndef _LOOP_RUNLOOP_H
#define _LOOP_RUNLOOP_H

#include "MessageLoop.h"

namespace loop {

    class MessageLoop;

    class RunLoop {
    public:
        RunLoop(MessageLoop* loop);

        ~RunLoop();

        void run();

        void run_until_idle();

        bool running() const { return m_running; }

        void quit();

        Closure quit_closure();

    private:
        friend class MessageLoop;

        MessageLoop* m_loop;

        volatile bool m_quit_called;
        volatile bool m_running;
    };

}

#endif