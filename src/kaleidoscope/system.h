// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <stdint.h>

// System Control
namespace kaleidoscope {
namespace hid {
namespace system {

class Report {
  friend class Dispatcher;
 public:
  Report();
  void clear();
  void add(byte keycode);
  void del(byte keycode);
 private:
  byte keycode_;
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
