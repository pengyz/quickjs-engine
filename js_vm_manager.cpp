#include "js_vm_manager.h"
#include "js_vm_environment.h"

#include <cassert>
#include <iostream>

#include <uv.h>
#include <libbf.h>
#include <quickjs.h>

js_vm_manager *js_vm_manager::_instance = nullptr;

js_vm_manager *js_vm_manager::get()
{
  if (!_instance)
  {
    _instance = new js_vm_manager();
  }
  return _instance;
}

std::shared_ptr<js_vm_environment> js_vm_manager::get_env(const std::string &token)
{
  if (!_vms.count(token))
    return nullptr;
  return std::get<0>(_vms.at(token));
}

bool js_vm_manager::create_js_env(const std::string &token)
{
  auto f = std::async(
      std::launch::async, [this](const std::string &token) -> int
      {
        //all struct must be created in the same thread.
        auto rt = JS_NewRuntime();
        assert(rt);
        auto ctx = JS_NewContext(rt);
        assert(ctx);
        auto vm_env = std::make_shared<js_vm_environment>(rt, ctx);
        bool bRet = vm_env->init_uv();
        if (!bRet)
        {
          return false;
        }
        //将启动的任务加入其中
        {
          std::lock_guard lck(vms_mutex);
          _vms[token] = std::make_tuple(vm_env, std::move(std::future<int>()));
        }
        return vm_env->run_loop();
      },
      token);
  //wait item add and update fucture
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool bContain = false;
    {
      std::lock_guard lck(vms_mutex);
      bContain = _vms.count(token);
    }
    if (bContain)
      break;
  }
  std::get<1>(_vms[token]) = std::move(f);

  return true;
}

bool js_vm_manager::stop_js_env(const std::string &token)
{
  if (!_vms.count(token))
    return false;
  auto p = std::move(_vms[token]);
  _vms.erase(token);
  auto [env, f] = std::move(p);
  env->stop_loop();
  auto ret = f.get();
  return !ret;
}