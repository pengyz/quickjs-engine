#include "gui.h"

#include <quickjs.h>
#include <cstddef>
#include <cassert>

std::pair<bool, rttr::variant> unwrap_js_value(JSContext *ctx, JSValueConst v, const rttr::parameter_info &info)
{
    rttr::variant ret;
    bool success = true;
    auto target_type = info.get_type();
    if (target_type == rttr::type::get<std::string>() || target_type == rttr::type::get<const char *>())
    {
        //string
        const char *str = "";
        if (JS_IsString(v))
        {
            str = JS_ToCString(ctx, v);
        }
        if (target_type == rttr::type::get<std::string>())
        {
            ret = rttr::variant(std::string(str));
        }
        else
        {
            ret = rttr::variant(str);
            success = false;
        }
    }
    else if (target_type == rttr::type::get<int32_t>() || rttr::type::get<uint32_t>())
    {
        int32_t r;
        if (!JS_ToInt32(ctx, &r, v))
        {
            success = false;
            r = 0;
        }
        ret = target_type == rttr::type::get<uint32_t>() ? rttr::variant(static_cast<uint32_t>(r)) : rttr::variant(r);
    }
    else if (target_type == rttr::type::get<int64_t>() || target_type == rttr::type::get<uint64_t>())
    {
        int64_t r;
        if (!JS_ToInt64(ctx, &r, v))
        {
            success = false;
            r = 0;
        }
        ret = target_type == rttr::type::get<uint64_t>() ? rttr::variant(static_cast<uint64_t>(r)) : rttr::variant(r);
    }
    else if (target_type == rttr::type::get<bool>())
    {
        bool b = false;
        if (!JS_IsBool(v))
        {
            success = false;
        }
        else
        {
            b = JS_VALUE_GET_BOOL(v);
        }
        ret = rttr::variant(b);
    }
    else if (target_type == rttr::type::get<double>() || target_type == rttr::type::get<float>())
    {
        double r = 0.0;
        if (!JS_ToFloat64(ctx, &r, v))
        {
            success = false;
            r = 0.0;
        }
        if (target_type == rttr::type::get<float>())
        {
            ret = rttr::variant(static_cast<float>(r));
        }
        else
        {
            ret = rttr::variant(r);
        }
    }
    else if (target_type == rttr::type::get<char>() || target_type == rttr::type::get<unsigned char>())
    {
        int32_t r;
        if (!JS_ToInt32(ctx, &r, v))
        {
            r = 0;
            success = false;
        }
        ret = target_type == rttr::type::get<unsigned char>() ? rttr::variant(static_cast<unsigned char>(r)) : rttr::variant(static_cast<char>(r));
    }
    else if (target_type == rttr::type::get<short>() || target_type == rttr::type::get<unsigned short>())
    {
        int32_t r;
        if (!JS_ToInt32(ctx, &r, v))
        {
            r = 0;
            success = false;
        }
        ret = target_type == rttr::type::get<unsigned short>() ? rttr::variant(static_cast<unsigned short>(r)) : rttr::variant(static_cast<short>(r));
    }
    else
    {
        success = false;
    }
    return std::make_pair(success, ret);
}

std::pair<bool, JSValue> wrap_js_value(JSContext *ctx, rttr::variant v)
{
    JSValue ret = JS_UNDEFINED;
    bool success = true;
    auto target_type = v.get_type();
    if (target_type == rttr::type::get<std::string>() || target_type == rttr::type::get<const char *>())
    {
        //string
        JS_NewString(ctx, target_type == rttr::type::get<std::string>() ? v.get_value<std::string>().c_str() : v.get_value<const char *>());
    }
    else if (target_type == rttr::type::get<int32_t>() || rttr::type::get<uint32_t>())
    {
        ret = target_type == rttr::type::get<uint32_t>() ? JS_NewUint32(ctx, v.get_value<uint32_t>()) : JS_NewInt32(ctx, v.get_value<int32_t>());
    }
    else if (target_type == rttr::type::get<int64_t>() || target_type == rttr::type::get<uint64_t>())
    {
        ret = target_type == rttr::type::get<uint64_t>() ? JS_NewBigUint64(ctx, v.get_value<uint64_t>()) : JS_NewBigInt64(ctx, v.get_value<int64_t>());
    }
    else if (target_type == rttr::type::get<bool>())
    {
        ret = JS_NewBool(ctx, v.get_value<bool>());
    }
    else if (target_type == rttr::type::get<double>() || target_type == rttr::type::get<float>())
    {
        ret = JS_NewFloat64(ctx, target_type == rttr::type::get<float>() ? static_cast<double>(v.get_value<float>()) : v.get_value<double>());
    }
    else if (target_type == rttr::type::get<char>() || target_type == rttr::type::get<unsigned char>())
    {
        ret = JS_NewInt32(ctx, target_type == rttr::type::get<unsigned char>() ? static_cast<int32_t>(v.get_value<unsigned char>())
                                                                               : static_cast<int32_t>(v.get_value<char>()));
    }
    else if (target_type == rttr::type::get<short>() || target_type == rttr::type::get<unsigned short>())
    {
        ret = JS_NewInt32(ctx, target_type == rttr::type::get<unsigned short>() ? static_cast<int32_t>(v.get_value<unsigned short>())
                                                                                : static_cast<int32_t>(v.get_value<char>()));
    }
    else
    {
        success = false;
    }
    return std::make_pair(success, ret);
}

#define FUNC_WRAPPER(module, method) g_module_c_func<module, method>
#define QJS_DEF_CFUNC(method, class, class_method) \
    JS_CFUNC_DEF(#method, (uint8_t)rttr::type::get<class>().get_method(#class_method).get_parameter_infos().size(), FUNC_WRAPPER(#class, #class_method))

RTTR_REGISTRATION
{
    //先注册rttr类型，再注册到js
    rttr::registration::class_<engine_gui_module>("engine_gui_module")
        .method("screenWidth", &engine_gui_module::screenWidth)
        .method("screenHeight", &engine_gui_module::screenHeight);
}

engine_gui_module *engine_gui_module::_instance = nullptr;

engine_gui_module *engine_gui_module::get()
{
    if (!_instance)
        _instance = new engine_gui_module();
    return _instance;
}

uint32_t engine_gui_module::screenWidth()
{
    return _screen_width;
}

uint32_t engine_gui_module::screenHeight()
{
    return _screen_height;
}

std::vector<JSCFunctionListEntry> engine_gui_module::exportModuleList()
{
    std::vector<JSCFunctionListEntry> list;
    #define func_1 g_module_c_func<"","">
    // JSCFunctionListEntry ent = JS_CFUNC_DEF("screenWidth", 0, func_1);
    JSCFunctionListEntry{ "screenWidth", JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, .u = { .func = { 0, JS_CFUNC_generic, { .generic = g_module_c_func<"",""> } } } };
    // JSCFunctionListEntry ent = QJS_DEF_CFUNC(screenWidth, engine_gui_module, screenWidth);
    return list;
}
