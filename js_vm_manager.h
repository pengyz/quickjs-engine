#pragma once
#include <memory>
#include <map>
#include <string>
#include <future>
#include <mutex>


struct JSRuntime;
struct JSContext;
class js_vm_environment;

class js_vm_manager
{
public:
  using js_vm_environment_ptr = std::shared_ptr<js_vm_environment>;

private:
  static js_vm_manager *_instance;
  std::mutex vms_mutex;
  std::map<std::string, std::tuple<js_vm_environment_ptr, std::future<int>>> _vms;

private:
  js_vm_manager() {}

public:
  static js_vm_manager *get();

  /**
   * @brief Get the env object
   * 
   * @param token 
   * @return std::shared_ptr<js_vm_environment> 
   */
  std::shared_ptr<js_vm_environment> get_env(const std::string &token = "default");
  /**
   * @brief Create a js env object
   * 
   * @param token   环境标识符，输入输出参数，如果给空则自动生成
   * @return true   创建成功
   * @return false  创建失败
   */
  bool create_js_env(const std::string &token = "default");
  /**
   * @brief 结束js环境，退出uv loop
   * 
   * @param token   环境标识符，用于索引js环境
   * @return true   成功退出
   * @return false  指定环境不存在或停止失败
   */
  bool stop_js_env(const std::string &token);
};
