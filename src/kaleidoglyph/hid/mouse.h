// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <PluggableUSB.h>

namespace kaleidoglyph {
namespace hid {
namespace mouse {

struct Report {
  byte buttons;
  int8_t x;
  int8_t y;
  int8_t v_wheel;
  int8_t h_wheel;
};


class Dispatcher {
 public:
  Dispatcher() {}
  void init();
  void move(int8_t x, int8_t y,
            int8_t v_wheel = 0, int8_t h_wheel = 0);
  void press(byte buttons);
  void release(byte buttons);
  void click(byte buttons);
  bool isPressed(byte buttons);
  byte pressedButtons();
 private:
  Report last_report_;

  void sendReport();
};

} // namespace mouse {
} // namespace hid {
} // namespace kaleidoscope {
