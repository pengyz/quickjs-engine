#pragma once

class main_window {
private:
  bool _visible;
  int _x;
  int _y;
  int _width;
  int _height;

public:
  main_window(int x, int y, int width, int height);
  void paint();
};
