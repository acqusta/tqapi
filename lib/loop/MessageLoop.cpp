//#include <apr_time.h>

#include "MessageLoop.h"
#include "RunLoop.h"
//#include <apr_atomic.h>

using namespace loop;

using namespace std;
using namespace std::chrono;

MessageLoop::MessageLoop() : m_run(nullptr)
{
    m_next_id = 1;
    //apr_pool_create(&m_pool, NULL);
    //apr_thread_mutex_create(&m_queue_lock, APR_THREAD_MUTEX_DEFAULT, m_pool);
    //apr_thread_cond_create(&m_queue_cond, m_pool);
};

MessageLoop::~MessageLoop()
{
    //apr_thread_mutex_destroy(m_queue_lock);
    //apr_pool_destroy(m_pool);
}

bool MessageLoop::DeleteTask(uint32_t id)
{
    unique_lock<mutex> lock(m_queue_lock);

    bool found = false;

    for (auto iter = m_queue.begin(); iter != m_queue.end(); iter++) {

        if (iter->id == id) {
            found = true;
            m_queue.erase(iter);
            break;
        }
    }

    return found;
}

uint32_t MessageLoop::PostTask(const Closure& task)
{
    return PostDelayedTask(task, 0);
}

uint32_t MessageLoop::PostDelayedTask(const Closure& task, uint32_t delay)
{
    unique_lock<mutex> lock(m_queue_lock);

    uint32_t id = m_next_id++;
    TaskInfo info(id, task, system_clock::now() + milliseconds(delay));

    m_queue.push_back(info);

    m_queue_cond.notify_all();

    return id;
}

void MessageLoop::QuitNow()
{
    if (m_run) m_run->Quit();
}

void MessageLoop::Run(RunLoop* run)
{
    m_run = run;
    while (!m_run->m_quit_called){

        TaskInfo task;
        bool found = false;

        {
            unique_lock<mutex> lock(m_queue_lock);

            auto now = system_clock::now();

            for (auto iter = m_queue.begin(); iter != m_queue.end(); iter++) {

                if (iter->execute_time <= now) {
                    task = *iter;
                    m_queue.erase(iter);
                    found = true;
                    break;
                }
            }

            if (!found) {
                m_queue_cond.wait_for(lock, milliseconds(100));
                continue;
            }
        }

        if (found)
            task.task();
    }

    m_run = nullptr;
}
