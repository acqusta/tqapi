#ifndef _LOOP_RUNLOOP_H
#define _LOOP_RUNLOOP_H

#include "MessageLoop.h"

namespace loop {

    class RunLoop {
    public:
        RunLoop(MessageLoop* loop);

        ~RunLoop();

        // Run the current MessageLoop. This blocks until Quit is called. Before
        // calling Run, be sure to grab an AsWeakPtr or the QuitClosure in order to
        // stop the MessageLoop asynchronously. MessageLoop::Quit and QuitNow will
        // also trigger a return from Run, but those are deprecated.
        void Run();

        // Run the current MessageLoop until it doesn't find any tasks or messages in
        // the queue (it goes idle). WARNING: This may never return! Only use this
        // when repeating tasks such as animated web pages have been shut down.
        void RunUntilIdle();

        bool running() const { return m_running; }

        // Quit an earlier call to Run(). There can be other nested RunLoops servicing
        // the same task queue (MessageLoop); Quitting one RunLoop has no bearing on
        // the others. Quit can be called before, during or after Run. If called
        // before Run, Run will return immediately when called. Calling Quit after the
        // RunLoop has already finished running has no effect.
        //
        // WARNING: You must NEVER assume that a call to Quit will terminate the
        // targetted message loop. If a nested message loop continues running, the
        // target may NEVER terminate. It is very easy to livelock (run forever) in
        // such a case.
        void Quit();

        // Convenience method to get a closure that safely calls Quit (has no effect
        // if the RunLoop instance is gone).
        //
        // Example:
        //   RunLoop run_loop;
        //   PostTask(run_loop.QuitClosure());
        //   run_loop.Run();
        Closure QuitClosure();

    private:
        friend class MessageLoop;

        // Return false to abort the Run.
        bool BeforeRun();
        void AfterRun();

        MessageLoop* m_loop;

        //// Parent RunLoop or NULL if this is the top-most RunLoop.
        //RunLoop* previous_run_loop_;

        //// Used to count how many nested Run() invocations are on the stack.
        //int run_depth_;

        //bool run_called_;
        volatile bool m_quit_called;
        volatile bool m_running;

        //// Used to record that QuitWhenIdle() was called on the MessageLoop, meaning
        //// that we should quit Run once it becomes idle.
        //bool quit_when_idle_received_;

        //// WeakPtrFactory for QuitClosure safety.
        ////base::WeakPtrFactory<RunLoop> weak_factory_;

        ////DISALLOW_COPY_AND_ASSIGN(RunLoop);
    };

}

#endif