#include "MessageLoop.h"
#include "RunLoop.h"

using namespace loop;

RunLoop::RunLoop(MessageLoop* loop) :
    m_loop(loop)
{
}

RunLoop::~RunLoop()
{

}
            // Run the current MessageLoop. This blocks until Quit is called. Before
            // calling Run, be sure to grab an AsWeakPtr or the QuitClosure in order to
            // stop the MessageLoop asynchronously. MessageLoop::Quit and QuitNow will
            // also trigger a return from Run, but those are deprecated.
void RunLoop::Run()
{
    m_quit_called = false;
    m_running = true;
    m_loop->Run(this);
    m_running = false;
}

            //// Run the current MessageLoop until it doesn't find any tasks or messages in
            //// the queue (it goes idle). WARNING: This may never return! Only use this
            //// when repeating tasks such as animated web pages have been shut down.
            //void RunUntilIdle();


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
void RunLoop::Quit()
{
    this->m_quit_called = true;
    //if (this->m_running)
}

            // Convenience method to get a closure that safely calls Quit (has no effect
            // if the RunLoop instance is gone).
            //
            // Example:
            //   RunLoop run_loop;
            //   PostTask(run_loop.QuitClosure());
            //   run_loop.Run();
Closure RunLoop::QuitClosure()
{
    return std::bind(&RunLoop::Quit, this);
}
