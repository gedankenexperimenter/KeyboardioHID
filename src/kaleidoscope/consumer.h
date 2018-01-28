// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <stdint.h>

// Consumer Control
namespace kaleidoscope {
namespace hid {
namespace consumer {

class Report {
 public:
  Report();
  void clear();
  void add(uint16_t keycode);
  void del(uint16_t keycode);
 private:
  static constexpr byte max_keycodes_ = 4;
  struct {
    uint16_t keycodes[max_keycodes_];
  } data_;
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
 private:
  Report last_report_;
  void sendReportUnchecked(const Report &report);
};

} // namespace consumer {
} // namespace hid {
} // namespace kaleidoscope {
