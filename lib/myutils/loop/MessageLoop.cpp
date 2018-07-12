#include <unordered_set>

#include "MessageLoop.h"
#include "RunLoop.h"

using namespace loop;

using namespace std;
using namespace std::chrono;

MessageLoop::MessageLoop() : m_run(nullptr)
{
    m_sem = myutils::SharedSemaphore::create(nullptr);
    m_next_id = 1;
};

MessageLoop::~MessageLoop()
{
    TaskInfo* task;
    while (m_quque.try_dequeue(task)) delete task;
    delete m_sem;
}

bool MessageLoop::delete_task(uint32_t id)
{
    m_deleted_queue.enqueue(id);
    return true;
}

uint32_t MessageLoop::post_task(const Closure& task)
{
    uint32_t id = m_next_id++;
    m_quque.enqueue(new TaskInfo(id, task, 0));
    m_sem->post();
    return id;
}

uint32_t MessageLoop::post_delayed_task(const Closure& task, uint32_t delay)
{
    uint32_t id = m_next_id++;
    int64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() + delay;
    m_quque.enqueue(new TaskInfo(id, task, time));
    m_sem->post();
    return id;
}

void MessageLoop::quit_now()
{
    if (m_run) m_run->quit();
}

void MessageLoop::run(RunLoop* run)
{
    m_run = run;

    list<TaskInfo*> delayed_queue;

    unordered_set<uint32_t> deleted_set;

    while (!m_run->m_quit_called){

        {
            uint32_t id;
            while (m_deleted_queue.try_dequeue(id)) deleted_set.insert(id);
        }

        int executed_count = 0;
        for (int i = 0; i < 100 && !m_run->m_quit_called; i++) {
            TaskInfo* task;
            if (!m_quque.try_dequeue(task)) break;
            if (task->execute_time)
                delayed_queue.push_back(task);
            else {
                task->task();
                delete task;
                executed_count++;
            }
        }

        if (!deleted_set.empty()) {
            for (auto iter = delayed_queue.begin(); iter != delayed_queue.end();) {
                if (deleted_set.find((*iter)->id) != deleted_set.end())
                    iter = delayed_queue.erase(iter);
                else
                    iter++;
            }
        }

        int64_t wait_time = 100; // 100 ms
        if (delayed_queue.size()) {
            int64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            int64_t next_execute_time = now + wait_time;
            for (auto iter = delayed_queue.begin(); iter != delayed_queue.end() && !m_run->m_quit_called;) {
                if ((*iter)->execute_time <= now) {
                    TaskInfo* task = *iter;
                    iter = delayed_queue.erase(iter);
                    executed_count++;
                    task->task();
                    delete task;
                }
                else {
                    if ((*iter)->execute_time < next_execute_time)
                        next_execute_time = (*iter)->execute_time;
                    iter++;
                }
            }

            wait_time = next_execute_time - now;
        }

        if (!executed_count)
            m_sem->timed_wait((int)wait_time);
    }

    for (TaskInfo* task : delayed_queue) delete task;

    m_run = nullptr;
}
