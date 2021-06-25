#include "mainwindow.h"
#include "js_vm_manager.h"
#include "js_vm_environment.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <quickjs.h>
#include <vector>

main_window::main_window(int x, int y, int width, int height)
    : _x(x), _y(y), _width(width), _height(height)
{
  bool bRet = js_vm_manager::get()->create_js_env();
  assert(bRet);
}

void main_window::paint()
{
  ImGui::SetNextWindowSize(ImVec2(_width, _height));
  ImGui::SetNextWindowPos(ImVec2(_x, _y));
  if (ImGui::Begin("MainWindow", &_visible))
  {
    //错误提示
    ImGui::SetNextWindowContentSize(ImVec2(200, 50));
    if (ImGui::BeginPopup("ErrorPopup"))
    {
      ImGui::Text("javascript execute failed!");
      if (ImGui::Button("ok"))
      {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::Text("Javascript code:");
    ImGui::SameLine();
    ImGui::InputText("##inputJavascript", &_javascript);
    ImGui::SameLine();
    if (ImGui::Button("Evecute") && _javascript.length())
    {
      auto env = js_vm_manager::get()->get_env();
      if (env)
      {
        bool bOk = env->eval(_javascript, "eval", 0, true);
        if (!bOk)
        {
          ImGui::OpenPopup("ErrorPopup");
        }
      }
    }
    ImGui::End();
  }
  else
  {
    _visible = false;
  }
}
