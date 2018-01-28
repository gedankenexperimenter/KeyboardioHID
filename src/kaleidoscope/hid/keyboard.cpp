// -*- c++ -*-

#include "Keyboard.h"

#include "DescriptorPrimitives.h"

#include <Arduino.h>

// New idea: separate the key report (data) from the dispatcher (HID report
// sender). This way, we can interact separately with the two, and even create
// new reports without having to save and restore one that's getting ready to be
// sent elsewhere. It also makes it possible to not bother with static storage
// of a full set of report data (29 bytes) all the time, and lets a Report
// object get passed as a parameter to pre-report hook functions without needing
// to involve the dispatcher.
namespace kaleidoscope {
namespace hid {
namespace keyboard {

static const byte descriptor[] PROGMEM = {
  //  NKRO Keyboard
  D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,
  D_USAGE, D_USAGE_KEYBOARD,
  D_COLLECTION, D_APPLICATION,
  D_REPORT_ID, HID_REPORTID_NKRO_KEYBOARD,
  D_USAGE_PAGE, D_PAGE_KEYBOARD,

  /* Key modifier byte */
  D_USAGE_MINIMUM, HID_KEYBOARD_FIRST_MODIFIER,
  D_USAGE_MAXIMUM, HID_KEYBOARD_LAST_MODIFIER,
  D_LOGICAL_MINIMUM, 0x00,
  D_LOGICAL_MAXIMUM, 0x01,
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, 0x08,
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),

  /* 5 LEDs for num lock etc, 3 left for advanced, custom usage */
  D_USAGE_PAGE, D_PAGE_LEDS,
  D_USAGE_MINIMUM, 0x01,
  D_USAGE_MAXIMUM, 0x08,
  D_REPORT_COUNT, 0x08,
  D_REPORT_SIZE, 0x01,
  D_OUTPUT, (D_DATA | D_VARIABLE | D_ABSOLUTE),

  // USB Code not within 4-49 (0x4-0x31), 51-155 (0x33-0x9B), 157-164 (0x9D-0xA4),
  // 176-221 (0xB0-0xDD) or 224-231 (0xE0-0xE7) NKRO Mode
  /* NKRO Keyboard */
  D_USAGE_PAGE, D_PAGE_KEYBOARD,

  // Padding 3 bits
  // To skip HID_KEYBOARD_NON_US_POUND_AND_TILDE, which causes
  // Linux to choke on our driver.
  D_REPORT_SIZE, 0x04,
  D_REPORT_COUNT, 0x01,
  D_INPUT, (D_CONSTANT),

  D_USAGE_MINIMUM, HID_KEYBOARD_A_AND_A,
  D_USAGE_MAXIMUM, HID_KEYBOARD_BACKSLASH_AND_PIPE,
  D_LOGICAL_MINIMUM, 0x00,
  D_LOGICAL_MAXIMUM, 0x01,
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, (HID_KEYBOARD_BACKSLASH_AND_PIPE - HID_KEYBOARD_A_AND_A)+1,
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),

  // Padding 1 bit.
  // To skip HID_KEYBOARD_NON_US_POUND_AND_TILDE, which causes
  // Linux to choke on our driver.
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, 0x01,
  D_INPUT, (D_CONSTANT),

  D_USAGE_MINIMUM, HID_KEYBOARD_SEMICOLON_AND_COLON,
  D_USAGE_MAXIMUM, HID_KEYBOARD_CANCEL,
  D_LOGICAL_MINIMUM, 0x00,
  D_LOGICAL_MAXIMUM, 0x01,
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, (HID_KEYBOARD_CANCEL-HID_KEYBOARD_SEMICOLON_AND_COLON) +1,
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),

  // Padding 1 bit.
  // To skip HID_KEYBOARD_CLEAR, which causes
  // Linux to choke on our driver.
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, 0x01,
  D_INPUT, (D_CONSTANT),

  D_USAGE_MINIMUM, HID_KEYBOARD_PRIOR,
  D_USAGE_MAXIMUM, HID_KEYPAD_HEXADECIMAL,
  D_LOGICAL_MINIMUM, 0x00,
  D_LOGICAL_MAXIMUM, 0x01,
  D_REPORT_SIZE, 0x01,
  D_REPORT_COUNT, (HID_KEYPAD_HEXADECIMAL - HID_KEYBOARD_PRIOR)  +1,
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),

  // Padding (w bits)
  D_REPORT_SIZE, 0x02,
  D_REPORT_COUNT, 0x01,
  D_INPUT, (D_CONSTANT),

  D_END_COLLECTION,
};

Dispatcher::Dispatcher() {
  static HIDSubDescriptor node(descriptor, sizeof(descriptor));
  HID().AppendDescriptor(&node);
}

// maybe we need separate begin() and end() methods?
void Dispatcher::init() {
  last_report_.clear();
  sendReportUnchecked(last_report_);
}

// I'm not at all convinced that it's worthwhile to check the return value, and
// the report-sending functions become more efficient if we just return void
// instead
int Dispatcher::sendReportUnchecked(const Report &report) {
  return HID().SendReport(HID_REPORTID_NKRO_KEYBOARD,
                          &report.data, sizeof(report.data));
}

// The while() loops are probably a bad idea. We should just make sure that any
// successful report sent will automatically reset things to a good state. If we
// can't send a report, nothing much matters, anyway.
bool Dispatcher::sendReport(const Report &report) {
  // chromeOS bug workaround
  if (report.mods() != last_report_.mods()) {
    last_report_.data_.modifiers = report.data_.modifiers;
    while (sendReportUnchecked(last_report_) < 0) {
      delay(10);
    }
  }
  // check different from last_report_;
  if (memcmp(last_report_.data_, report.data_, sizeof(report.data_))) {
    while (sendReportUnchecked(report) < 0) {
      delay(10);
    }
    memcpy(last_report_.data_, report.data_, sizeof(report.data_));
  }
  return true;
}

byte Dispatcher::ledState() {
  return HID().getLEDs();
}

// Maybe it's better to just return a reference to the last report? The problem
// there is that it could then be modified.
byte Dispatcher::lastModifierState() {
  return last_report_.mods();
}


// Do I need to initialize to zeros? If so, how?
Report::Report() {
  this->clear();
}

void Report::clear() {
  memset(&data_, 0, sizeof(data_));
}

void Report::setKeycode(byte keycode, bool val) {
  if (keycode <= HID_LAST_KEY) {
    // Normal, printable keys (plus some that shouldn't be in here)
    byte idx = keycode / 8;
    byte bit = keycode % 8;
    byte mask = 1 << bit;
    if (val) {
      data_.keycodes[idx] |= mask;
    } else {
      data_.keycodes[idx] &= ~mask;
    }
  } else if (k >= HID_KEYBOARD_FIRST_MODIFIER &&
             k <= HID_KEYBOARD_LAST_MODIFIER) {
    // Modifier keys
    byte mask = 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    if (v) {
      data_.modifiers |= mask;
    } else {
      data_.modifiers &= ~mask;
    }
  }
  // There are unknown keycodes; we just ignore them silently
}

// This method is of dubious value
bool Report::isActive(byte keycode) {
  byte idx = keycode / 8;
  byte bit = keycode % 8;
  byte mask = 1 << bit;
  // do I need the bool() here?
  return bool(data_.keycodes[idx] & mask);
}

} // namespace keyboard {
} // namespace hid {
} // namespace kaleidoscope {
