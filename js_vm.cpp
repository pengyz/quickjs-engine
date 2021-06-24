#include "js_vm.h"
#include "third_party/quickjs/src/libbf.h"
#include <quickjs.h>
#include <cassert>
#include <iostream>

js_vm* js_vm::_instance = nullptr;


js_vm* js_vm::get()
{
  if(!_instance) {
    _instance = new js_vm();
  }
  return _instance;
}

bool js_vm::init()
{
  _rt = JS_NewRuntime();
  assert(_rt);
  _ctx = JS_NewContext(_rt);
  assert(_ctx);

  return true;
}


bool js_vm::eval(const char *buffer, size_t len, const char* filename, int flags)
{
  auto v = JS_Eval(_ctx, buffer, len, filename, flags);
  if(JS_IsException(v)) {
    auto stackObj = JS_GetPropertyStr(_ctx, v, "stack");
    if(!JS_IsUndefined(stackObj)) {
      auto str = JS_ToCString(_ctx, stackObj);
      if(str) {
        std::cout << "error:" << str << std::endl;
      }else {
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
