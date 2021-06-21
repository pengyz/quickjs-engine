#include "mainwindow.h"
#include <imgui.h>

main_window::main_window(int x, int y, int width, int height)
    : _x(x), _y(y), _width(width), _height(height) {}

void main_window::paint()
{
  ImGui::SetNextWindowSize(ImVec2(_width, _height));
  ImGui::SetNextWindowPos(ImVec2(_x,_y));
  if(ImGui::Begin("MainWindow", &_visible)) {

    ImGui::End();
  }
}
