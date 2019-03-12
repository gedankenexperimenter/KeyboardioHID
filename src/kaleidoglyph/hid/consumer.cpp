// -*- c++ -*-
#if 0
#include "consumer.h"

#include "DescriptorPrimitives.h"

#include <Arduino.h>
#include <stdint.h>


// Consumer Control
namespace kaleidoscope {
namespace hid {
namespace consumer {

static const byte descriptor[] PROGMEM = {
  // Consumer Control (Sound/Media keys)
  D_USAGE_PAGE, 0x0C,				/* usage page (consumer device) */
  D_USAGE, 0x01, 				/* usage -- consumer control */
  D_COLLECTION, D_APPLICATION, 			/* collection (application) */
  D_REPORT_ID, HID_REPORTID_CONSUMERCONTROL, 	/* report id */
  // 4 Media Keys
  D_LOGICAL_MINIMUM, 0x00, 			/* logical minimum */
  D_MULTIBYTE(D_LOGICAL_MAXIMUM), 0xFF, 0x03, 	/* logical maximum (3ff) */
  D_USAGE_MINIMUM, 0x00, 			/* usage minimum (0) */
  D_MULTIBYTE(D_USAGE_MAXIMUM), 0xFF, 0x03, 	/* usage maximum (3ff) */
  D_REPORT_COUNT, 0x04, 			/* report count (4) */
  D_REPORT_SIZE, 0x10, 				/* report size (16) */
  D_INPUT, 0x00, 				/* input */
  D_END_COLLECTION 				/* end collection */
};

Report::Report() {
  this->clear();
}

void Report::clear() {
  memset(&data_, 0, sizeof(data_));
}

void Report::add(uint16_t keycode) {
  for (byte i = 0; i < max_keycodes_; ++i) {
    if (data_.keycodes[i] == 0) {
      data_.keycodes[i] = keycode;
      break;
    }
  }
}

void Report::del(uint16_t keycode) {
  for (byte i = 0; i < max_keycodes_; ++i) {
    if (data_.keycodes[i] == keycode) {
      data_.keycodes[i] = 0;
      // no break; delete all instances of this keycode from the report
    }
  }
}


Dispatcher::Dispatcher() {
  static HIDSubDescriptor node(descriptor, sizeof(descriptor));
  HID().AppendDescriptor(&node);
};

void Dispatcher::init() {
  last_report_.clear();
  sendReportUnchecked(last_report_);
}

void Dispatcher::sendReportUnchecked(const Report& report) {
  HID().SendReport(HID_REPORTID_CONSUMERCONTROL, &report.data_, sizeof(report.data_));
}

void Dispatcher::sendReport(const Report& report) {
  // If the last report is different than the current report, then we need to send a report.
  // We guard sendReport like this so that calling code doesn't end up spamming the host with empty reports
  // if sendReport is called in a tight loop.

  // if the previous report is the same, return early without a new report.
  if (memcmp(&last_report_.keycodes_, &report.data_, sizeof(report.data_)) == 0)
    return;

  sendReportUnchecked(report);
  memcpy(&_lastReport, &_report, sizeof(_report));
}

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
#endif
