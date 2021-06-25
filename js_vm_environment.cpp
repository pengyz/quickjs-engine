#include <uv.h>
#include "js_vm_environment.h"

#include <cassert>
#include <iostream>

#include <quickjs.h>

RTTR_REGISTRATION
{
    rttr::registration::class_<js_vm_environment>("js_vm_environment")
        .method("eval", &js_vm_environment::__eval, rttr::access_levels::private_access);
}

void notify_worker::push(const std::shared_ptr<notify_event> notify)
{
    std::lock_guard<std::mutex> lck(event_mutex);
    event_queue.emplace(notify);
    uv_async_send(async);
}
//pop item
std::shared_ptr<notify_event> notify_worker::pop()
{
    std::lock_guard<std::mutex> lck(event_mutex);
    auto item = std::move(event_queue.front());
    event_queue.pop();
    return item;
}
//clear all items
void notify_worker::clear()
{
    std::lock_guard<std::mutex> lck(event_mutex);
    while (!event_queue.empty())
    {
        event_queue.pop();
    }
}

bool notify_worker::empty()
{
    std::lock_guard<std::mutex> lck(event_mutex);
    return event_queue.empty();
}

js_vm_environment::js_vm_environment(JSRuntime *rt, JSContext *ctx)
    : _rt(rt), _ctx(ctx)
{
    assert(_rt);
    assert(_ctx);
}

js_vm_environment::~js_vm_environment()
{
    if (_ctx)
    {
        JS_FreeContext(_ctx);
        _ctx = nullptr;
    }
    if (_rt)
    {
        JS_FreeRuntime(_rt);
        _rt = nullptr;
    }

    delete _uv_loop;
    _uv_loop = nullptr;
}

uv_loop_t *js_vm_environment::get_loop()
{
    return _uv_loop;
}

JSRuntime *js_vm_environment::get_runtime()
{
    return _rt;
}

void js_vm_environment::uv_prepare_handler(uv_prepare_t *handler)
{
    auto env = static_cast<js_vm_environment *>(handler->data);
    __uv_maybe_idle(env);
}

void js_vm_environment::uv_check_handler(uv_check_t *handler)
{
    js_vm_environment *env = static_cast<js_vm_environment *>(handler->data);
    JSContext *ctx;
    for (;;)
    {
        int err = JS_ExecutePendingJob(env->_rt, &ctx);
        if (err <= 0)
        {
            if (err < 0)
            {
                //dump error?
            }
            break;
        }
    }
    __uv_maybe_idle(env);
}

void js_vm_environment::uv_async_handler(uv_async_t *async)
{
    auto env = static_cast<js_vm_environment *>(async->data);
    while (!env->notify.empty())
    {
        auto evt = env->notify.pop();

        auto env_class = rttr::type::get<decltype(env)>();
        auto target_function = env_class.get_method(evt->function);
        if (target_function.is_valid())
        {
            //create args, rttr::argument only hold the rttr::variant ptr.
            std::vector<rttr::argument> args;
            for(int i=0;i<evt->params.size();i++) {
                args.emplace_back(evt->params[i]);
            }
            //invoke the method
            auto ret = env_class.invoke(evt->function, env, args);
            if (evt->sync)
            {
                evt->p.set_value(ret);
            }
        }
    }
}

void js_vm_environment::__uv_maybe_idle(js_vm_environment *env)
{
    if (JS_IsJobPending(env->get_runtime()))
    {
        //启动idle
        uv_idle_start(env->jobs.idle, [](uv_idle_t *handle) { /*do nothing.*/std::cout << "idle..." << std::endl; });
    }
    else
    {
        //停止idle
        uv_idle_stop(env->jobs.idle);
    }
}

bool js_vm_environment::__eval(const std::string &javascript, const std::string &filename, int flags)
{
    auto v = JS_Eval(_ctx, javascript.c_str(), javascript.length(), filename.c_str(), flags);
    if (JS_IsException(v))
    {
        auto stackObj = JS_GetPropertyStr(_ctx, v, "stack");
        if (!JS_IsUndefined(stackObj))
        {
            auto str = JS_ToCString(_ctx, stackObj);
            if (str)
            {
                std::cout << "error:" << str << std::endl;
            }
            else
            {
                std::cout << "[Exception]" << std::endl;
            }
            JS_FreeCString(_ctx, str);
        }
        JS_FreeValue(_ctx, v);
        return false;
    }
    JS_FreeValue(_ctx, v);
    return true;
}

bool js_vm_environment::init_uv()
{
    _uv_loop = new uv_loop_t();
    int ret = uv_loop_init(_uv_loop);
    if (ret)
    {
        std::cout << "uv_loop_init failed: " << uv_strerror(ret) << std::endl;
        return false;
    }

    jobs.check = new uv_check_t();
    ret = uv_check_init(_uv_loop, jobs.check);
    if (ret)
    {
        std::cout << "uv_check_init failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    jobs.check->data = this;

    jobs.prepare = new uv_prepare_t();
    ret = uv_prepare_init(_uv_loop, jobs.prepare);
    if (ret)
    {
        std::cout << "uv_prepare_init failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    jobs.prepare->data = this;

    jobs.idle = new uv_idle_t();
    ret = uv_idle_init(_uv_loop, jobs.idle);
    if (ret)
    {
        std::cout << "uv_idle_init failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    jobs.idle->data = this;

    //init notify
    notify.async = new uv_async_t();
    ret = uv_async_init(_uv_loop, notify.async, uv_async_handler);
    if (ret)
    {
        std::cout << "uv_async_init failed: " << uv_strerror(ret) << std::endl;
        return false;
    }
    notify.async->data = this;
    return true;
}

int js_vm_environment::run_loop()
{
    int ret = uv_prepare_start(jobs.prepare, uv_prepare_handler);
    if (ret)
    {
        std::cout << "uv_prepare_start failed: " << uv_strerror(ret) << std::endl;
        return ret;
    }
    uv_unref((uv_handle_t *)jobs.prepare);
    ret = uv_check_start(jobs.check, uv_check_handler);
    if (ret)
    {
        std::cout << "uv_check_start failed: " << uv_strerror(ret) << std::endl;
        return ret;
    }
    uv_unref((uv_handle_t *)jobs.check);
    __uv_maybe_idle(this);
    ret = uv_run(get_loop(), UV_RUN_DEFAULT);
    if (ret)
    {
        std::cout << "uv_run failed: " << uv_strerror(ret) << std::endl;
        return ret;
    }
    return ret;
}

void js_vm_environment::stop_loop()
{
    if (uv_loop_alive(_uv_loop))
        uv_stop(_uv_loop);
    uv_loop_close(_uv_loop);
}

bool js_vm_environment::eval(const std::string &javascript, const std::string &filename, int flags, bool sync)
{
    auto evt = std::make_shared<notify_event>();
    evt->function = "eval";
    evt->sync = sync;
    if (sync)
        evt->p = std::promise<rttr::variant>();
    evt->params.emplace_back(rttr::variant(javascript));
    evt->params.emplace_back(rttr::variant(filename));
    evt->params.emplace_back(rttr::variant(flags));
    notify.push(evt);
    bool bRet = true;
    if (sync)
    {
        bRet = evt->p.get_future().get().to_bool();
    }
    return bRet;
}
