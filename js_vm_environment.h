#pragma once
#include <vector>
#include <future>
#include <variant>
#include <queue>
#include <rttr/registration>
#include <rttr/registration_friend>

//declear them, do not include uv.h in header file
typedef struct uv_loop_s uv_loop_t;
typedef struct uv_check_s uv_check_t;
typedef struct uv_prepare_s uv_prepare_t;
typedef struct uv_async_s uv_async_t;
typedef struct uv_idle_s uv_idle_t;

struct JSRuntime;
struct JSContext;

enum class notify_type
{
    none,
    eval_js_code,
    load_js_file,
};

struct notify_event
{
    std::string function;
    std::vector<rttr::variant> params;
    bool sync = false;
    std::promise<rttr::variant> p;
};

struct notify_worker
{
    uv_async_t *async = nullptr;
    std::mutex event_mutex;
    std::queue<std::shared_ptr<notify_event>> event_queue;

    void push(const std::shared_ptr<notify_event> notify);
    std::shared_ptr<notify_event> pop();
    void clear();
    bool empty();
};

/**
 * @brief JS虚拟机环境管理类
 * 这个类会在子线程中初始化整个JS环境，事件通知基于libuv异步执行
 * 
 */
using namespace rttr;
class js_vm_environment
{
private:
    RTTR_REGISTRATION_FRIEND
    uv_loop_t *_uv_loop = nullptr;
    //uv handler
    struct
    {
        uv_prepare_t *prepare = nullptr;
        uv_idle_t *idle = nullptr;
        uv_check_t *check = nullptr;
    } jobs;
    notify_worker notify;
    JSRuntime *_rt = nullptr;
    JSContext *_ctx = nullptr;

private:
    static void uv_prepare_handler(uv_prepare_t *handler);
    static void uv_check_handler(uv_check_t *handler);
    static void uv_async_handler(uv_async_t *async);
    static void __uv_maybe_idle(js_vm_environment *env);

private:
    bool __eval(const std::string &javascript, const std::string &filename, int flags);

public:
    js_vm_environment(JSRuntime *_rt, JSContext *_ctx);
    ~js_vm_environment();
    uv_loop_t *get_loop();
    JSRuntime *get_runtime();
    /**
   * @brief 初始化uv事件循环
   * 
   * @return true 
   * @return false 
   */
    bool init_uv();
    /**
   * @brief 启动uv事件循环
   * 
   * @return int 
   */
    int run_loop();

    /**
     * @brief 结束uv事件循环
     * 
     */
    void stop_loop();
    /**
   * @brief 执行js函数
   * 
   * @param javascript  js代码
   * @param filename    文件名
   * @param flags       flag标记
   * @return true       成功
   * @return false      失败
   */
    bool eval(const std::string &javascript, const std::string &filename, int flags, bool sync = false);
};
