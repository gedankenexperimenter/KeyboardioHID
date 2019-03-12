// -*- c++ -*-

#pragma once

#include <Arduino.h>
#include <HID.h>

// New idea: separate the key report (data) from the dispatcher (HID report
// sender). This way, we can interact separately with the two, and even create
// new reports without having to save and restore one that's getting ready to be
// sent elsewhere. It also makes it possible to not bother with static storage
// of a full set of report data (29 bytes) all the time, and lets a Report
// object get passed as a parameter to pre-report hook functions without needing
// to involve the dispatcher.
namespace kaleidoglyph {
namespace hid {
namespace keyboard {

constexpr byte keycode_bytes = bitfieldSize(HID_KEYBOARD_FIRST_MODIFIER);

// struct BootReport {
//   byte data[8] = {};
// };

class Report {
  friend class Dispatcher;
 public:
  Report();
  void clear();
  void addKeycode(byte keycode);
  void removeKeycode(byte keycode);
  byte getModifiers() const { return data_[0]; }
  void setModifiers(byte mods) { data_[0] = mods; }
  // Do I want a function to get the state of any given keycode?
  bool readKeycode(byte keycode);

  //BootReport getBootReport();

  bool operator==(const Report& other) const {
    // Is memcmp really faster?
    return memcmp(data_, other.data_, sizeof(data_));
    // for (byte i{0}; i < arraySize(data_); ++i) {
    //   if (data_[i] != other.data_[i])
    //     return false;
    // }
    // return true;
  }
  bool operator!=(const Report& other) const {
    return !(*this == other);
  }

 private:
  // It's important to make this a single array, rather than a struct with a separate
  // modifiers byte and keycodes array, because we're going to send this report data
  // directly to the HID().sendReport() function, and this is the only way to guarantee
  // that there won't be any padding bytes between the two members.
  byte data_[1 + keycode_bytes] = {};

  bool updatePlainReleases_(const Report& new_report);

  void updateFrom_(const Report& other) {
    // Is memcpy really faster?
    memcpy(data_, other.data_, sizeof(data_));
    // for (byte i{0}; i < arraySize(data_); ++i) {
    //   data_[i] = other.data_[i];
    // }
  }
};


class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
  byte getLedState() const;
  byte lastModifierState() const;
 private:
  Report last_report_;
  bool boot_protocol_{false};
  void sendReportUnchecked_(const Report &report);
};


} // namespace keyboard {
} // namespace hid {
} // namespace kaleidoscope {
