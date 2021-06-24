#pragma once
#include <cstddef>


struct JSRuntime;

class js_vm {
 private:
  static js_vm* _instance;
  struct JSRuntime* _rt = nullptr;
  struct JSContext* _ctx = nullptr;
 private:
  js_vm() {}

public:
  static js_vm *get();
  bool init();
  bool eval(const char* buffer, size_t len, const char* filename, int flags);


};
