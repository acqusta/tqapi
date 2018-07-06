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
void RunLoop::run()
{
    m_quit_called = false;
    m_running = true;
    m_loop->run(this);
    m_running = false;
}

void RunLoop::quit()
{
    this->m_quit_called = true;
    //if (this->m_running)
}

Closure RunLoop::quit_closure()
{
    return std::bind(&RunLoop::quit, this);
}
