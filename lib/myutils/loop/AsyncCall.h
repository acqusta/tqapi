#ifndef _LOOP_ASYNCCALL_H
#define _LOOP_ASYNCCALL_H

#include <memory>
#include <functional>
#include "MessageLoop.h"
#include "RunLoop.h"

namespace loop {
    using std::shared_ptr;
    using std::function;

    template<class T_Req, class T_Rsp>
    struct AsyncCallResult {
        shared_ptr<T_Req> req;
        shared_ptr<T_Rsp> rsp;
    };

    template <class T_Req, class T_Rsp>
    void asyncCallResult(MessageLoop* loop, shared_ptr < AsyncCallResult<T_Req, T_Rsp> > result, shared_ptr<T_Rsp> rsp)
    {
        result->rsp = rsp;
        loop->quit_now();
    }

    template <class T_Req, class T_Rsp>
    void notifyAysnCallResult(shared_ptr<MessageLoop> loop, shared_ptr<AsyncCallResult<T_Req, T_Rsp> > result, shared_ptr<T_Rsp> rsp)
    {
        loop->post_task(std::bind(&asyncCallResult<T_Req, T_Rsp>, loop.get(), result, rsp));
    }

    //template <class T_Req, class T_Rsp>
    //shared_ptr<T_Rsp> aync_call(
    //    function<void(shared_ptr<T_Req>, std::function< void(shared_ptr<T_Rsp>) >) > func,
    //    shared_ptr<T_Req> req)
    //{
    //    shared_ptr<MessageLoop> loop(new MessageLoop());
    //    auto asyncCallResult = shared_ptr<AsyncCallResult<T_Req, T_Rsp> >(new AsyncCallResult<T_Req, T_Rsp>());
    //    std::function< void(shared_ptr<T_Rsp>)> callback = std::bind(&notifyAysnCallResult<T_Req, T_Rsp>, loop, asyncCallResult, std::placeholders::_1);
    //    loop->post_task(std::bind(func, req, callback));
    //    RunLoop run(loop.get());
    //    run.Run();
    //    return asyncCallResult->rsp;
    //}

    template <class T_Req, class T_Rsp>
    shared_ptr<T_Rsp> aync_call(
        function<void(shared_ptr<T_Req>, std::function< void(shared_ptr<T_Rsp>) >*) > func,
        shared_ptr<T_Req> req)
    {
        shared_ptr<MessageLoop> loop(new MessageLoop());
        auto asyncCallResult = shared_ptr<AsyncCallResult<T_Req, T_Rsp> >(new AsyncCallResult<T_Req, T_Rsp>());
        std::function< void(shared_ptr<T_Rsp>)> callback = std::bind(&notifyAysnCallResult<T_Req, T_Rsp>, loop, asyncCallResult, std::placeholders::_1);
        loop->post_task(std::bind(func, req, &callback));
        RunLoop(loop.get()).run();
        return asyncCallResult->rsp;
    }

}

#endif
