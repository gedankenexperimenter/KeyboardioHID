// -*- c++ -*-

#include "kaleidoglyph/hid/keyboard.h"

#include <Arduino.h>

#include "DescriptorPrimitives.h"


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

static constexpr PROGMEM byte nkro_descriptor[] = {
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

// See Appendix B of USB HID spec
static const PROGMEM byte boot_descriptor[] = {
  //  Keyboard
  D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,
  D_USAGE, D_USAGE_KEYBOARD,

  D_COLLECTION, D_APPLICATION,
  // Modifiers
  D_USAGE_PAGE, D_PAGE_KEYBOARD,
  D_USAGE_MINIMUM, 0xe0,
  D_USAGE_MAXIMUM, 0xe7,
  D_LOGICAL_MINIMUM, 0x0,
  D_LOGICAL_MAXIMUM, 0x1,
  D_REPORT_SIZE, 0x1,
  D_REPORT_COUNT, 0x8,
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),

  // Reserved byte
  D_REPORT_COUNT, 0x1,
  D_REPORT_SIZE, 0x8,
  D_INPUT, (D_CONSTANT),

  // LEDs
  D_REPORT_COUNT, 0x5,
  D_REPORT_SIZE, 0x1,
  D_USAGE_PAGE, D_PAGE_LEDS,
  D_USAGE_MINIMUM, 0x1,
  D_USAGE_MAXIMUM, 0x5,
  D_OUTPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),
  // Pad LEDs up to a byte
  D_REPORT_COUNT, 0x1,
  D_REPORT_SIZE, 0x3,
  D_OUTPUT, (D_CONSTANT),

  // Non-modifiers
  D_REPORT_COUNT, 0x6,
  D_REPORT_SIZE, 0x8,
  D_LOGICAL_MINIMUM, 0x0,
  D_LOGICAL_MAXIMUM, 0xff,
  D_USAGE_PAGE, D_PAGE_KEYBOARD,
  D_USAGE_MINIMUM, 0x0,
  D_USAGE_MAXIMUM, 0xff,
  D_INPUT, (D_DATA|D_ARRAY|D_ABSOLUTE),
  D_END_COLLECTION
};



Dispatcher::Dispatcher() {
  static HIDSubDescriptor node(nkro_descriptor, sizeof(nkro_descriptor));
  HID().AppendDescriptor(&node);
}

// maybe we need separate begin() and end() methods?
void Dispatcher::init() {
  last_report_.clear();
  sendReportUnchecked_(last_report_);
}

// I'm not at all convinced that it's worthwhile to check the return value, and
// the report-sending functions become more efficient if we just return void
// instead
int Dispatcher::sendReportUnchecked_(const Report &report) {
  // if (boot_protocol_) {
  //   BootReport boot_report = report.getBootReport();
  //   return USB_Send(pluggedEndPoint | TRANSFER_RELEASE, &boot_report, sizeof(boot_report));
  // }
  return HID().SendReport(HID_REPORTID_NKRO_KEYBOARD,
                          &report.data_, sizeof(report.data_));
}

// Convert NKRO report data to boot protocol (6KRO) report format
// BootReport Report::getBootReport() const {
//   BootReport boot_report;
//   byte next_keycode_index{0};
//   // modifiers
//   boot_report.data[0] = getModifiers();
//   for (byte i{1}; i < arraySize(data_); ++i) {
//     if (data_[i] == 0) continue;
//     for (byte n{0}; n < 8; ++n) {
//       if (bitRead(data_[i], n)) {
//         byte keycode = ((i - 1) * 8) + n;
//         boot_report.data[2 + next_keycode_index] = keycode;
//         ++next_keycode_index;
//         if (next_keycode_index > 5) {
//           return boot_report;
//         }
//       }
//     }
//   }
//   return boot_report;
// }

// Update the report to remove any keycodes that don't appear in `new_report`. If any such
// keycodes are found, return `true`, otherwise return `false`. This efficiently deals
// with the problem of a plain keycode release in the same update as a new modifier
// press. We don't want to risk the modifier applying to the plain keycode before that
// keycode is released.
bool Report::updatePlainReleases_(const Report &new_report) {
  bool result{false};
  for (byte i{1}; i < arraySize(data_); ++i) {
    byte released_keycodes = data_[i] & ~(new_report.data_[i]);
    if (released_keycodes != 0) {
      data_[i] &= ~released_keycodes;
      result = true;
    }
  }
  return result;
}


// 1. mod_press -> plain_press
// 2. plain_release -> mod_release
// 3. plain_release -> mod_press
// plain_release -> mod_press -> plain_press -> mod_release
bool Dispatcher::sendReport(const Report &new_report) {

  // First, we determine if any modifiers have changed state
  byte new_modifiers = new_report.getModifiers();
  byte old_modifiers = last_report_.getModifiers();
  byte changed_modifiers = new_modifiers ^ old_modifiers;
  byte pressed_modifiers = changed_modifiers & new_modifiers;
  byte released_modifiers = changed_modifiers & old_modifiers;

  // If any modifiers were released, send any release events for plain keycodes so that
  // any modifiers that toggled on in the same report can't be applied to those released
  // keycodes.
  if ((released_modifiers != 0) &&
      last_report_.updatePlainReleases_(new_report)) {
    sendReportUnchecked_(last_report_);
  }

  // Next, if any modifiers were added since the previous report, we need to send those
  // first to ensure that the modifiers will be applied before any keycodes that were
  // added in the same report.
  if (pressed_modifiers != 0) {
    last_report_.setModifiers(old_modifiers | pressed_modifiers);
    sendReportUnchecked_(last_report_);
  }

  // Next, we send any keycodes that differ between this report and the previous one,
  // using the modifiers from the previous report (which, at this point, might be the one
  // we just sent above with the added modifiers).
  new_report.setModifiers(last_report_.getModifiers());
  if (new_report != last_report_) {
    sendReportUnchecked_(new_report);
  }

  // Finally, if any modifiers were released, we send those in a separate report to
  // prevent them from being processed before the release of the other keycodes and
  // causing spurious output.  For example, releasing `shift` and `3` in the same report
  // after holding both keys long enough to repeat can result in `####3`.
  if (released_modifiers != 0) {
    new_report.setModifiers(new_modifiers);
    sendReportUnchecked_(new_report);
  }

  // We're done, so we record the last report that was sent for use next time.
  last_report_.updateFrom_(new_report);
}

byte Dispatcher::getLedState() const {
  return HID().getLEDs();
}

// Maybe it's better to just return a reference to the last report? The problem
// there is that it could then be modified.
byte Dispatcher::lastModifierState() const {
  return last_report_.mods();
}


// Do I need to initialize to zeros? If so, how?
Report::Report() {
  this->clear();
}

void Report::clear() {
  memset(&data_, 0, sizeof(data_));
}

void Report::addKeycode(byte keycode) {
  byte n = keycode / 8;
  byte i = keycode % 8;
  if (keycode < HID_KEYBOARD_FIRST_MODIFIER) {
    bitSet(data_[1 + n], i);
  } else if (keycode <= HID_KEYBOARD_LAST_MODIFIER) {
    bitSet(data_[0], i);
  }
}

void Report::removeKeycode(byte keycode) {
  byte n = keycode / 8;
  byte i = keycode % 8;
  if (keycode < HID_KEYBOARD_FIRST_MODIFIER) {
    bitClear(data_[1 + n], i);
  } else if (keycode <= HID_KEYBOARD_LAST_MODIFIER) {
    bitClear(data_[0], i);
  }
}

// This method is of dubious value
bool Report::readKeycode(byte keycode) {
  byte n = keycode / 8;
  byte i = keycode % 8;
  if (keycode < HID_KEYBOARD_FIRST_MODIFIER) {
    bitRead(data_[1 + n], i);
  } else if (keycode <= HID_KEYBOARD_LAST_MODIFIER) {
    bitRead(data_[0], i);
  }
}

} // namespace keyboard {
} // namespace hid {
} // namespace kaleidoscope {
