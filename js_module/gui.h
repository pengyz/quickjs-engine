#pragma once
#include <string>
#include <cstddef>
#include <quickjs.h>
#include <rttr/registration>
#include <cassert>

class js_vm_environment;

std::pair<bool, rttr::variant> unwrap_js_value(JSContext *ctx, JSValueConst v, const rttr::parameter_info &info);
std::pair<bool, JSValue> wrap_js_value(JSContext *ctx, rttr::variant v);

class js_module_base
{
public:
    virtual std::vector<JSCFunctionListEntry> exportModuleList() = 0;
};

template <const char *class_name, const char *method_name>
static JSValue g_module_c_func(JSContext *ctx, JSValue this_val, int argc, JSValue *argv)
{
    auto methods = rttr::type::get_by_name("class_name").get_method("method_name");
    if (!methods.is_valid())
        return JS_UNDEFINED;
    auto param_infos = methods.get_parameter_infos();
    int i = 0;
    std::vector<rttr::variant> call_param;
    call_param.reserve(param_infos.size());
    std::vector<rttr::argument> call_args;
    call_args.reserve(param_infos.size());
    for (const auto &info : param_infos)
    {
        JSValueConst v = i < argc ? argv[i] : JS_UNDEFINED;
        auto [success, var] = unwrap_js_value(ctx, v, info);
        if (!var.is_valid() || var.get_type() != info.get_type())
        {
            return JS_UNDEFINED;
        }
        call_param.push_back(var);
        call_args.push_back(rttr::argument(call_param[call_param.size() - 1]));
        i++;
    }
    assert(call_param.size() == param_infos.size());
    //执行
    rttr::variant ret = methods.invoke(rttr::variant(), call_args);
    assert(methods.get_return_type() == ret.get_type());
    auto [success, js_ret] = wrap_js_value(ctx, ret);
    return success ? js_ret : JS_UNDEFINED;
}
/**
 * @brief 引擎的gui模块，绑定到js供js调用
 * 
 */
class engine_gui_module : public js_module_base
{
private:
    static engine_gui_module *_instance;
    engine_gui_module() = default;

public:
    static engine_gui_module *get();

    //注册接口
public:
    uint32_t screenWidth();
    uint32_t screenHeight();

    virtual std::vector<JSCFunctionListEntry> exportModuleList();

private:
    uint32_t _screen_width = 0;
    uint32_t _screen_height = 0;
};