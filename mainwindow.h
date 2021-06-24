#pragma once
#include <string>

class main_window {
private:
  bool _visible;
  int _x;
  int _y;
  int _width;
  int _height;
  std::string _javascript;

public:
  main_window(int x, int y, int width, int height);
  /**
   * @brief 主界面的绘制代码
   * 
   */
  void paint();
};
