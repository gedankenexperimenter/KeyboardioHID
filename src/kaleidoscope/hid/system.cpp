// -*- c++ -*-

#include "system.h"

#include "DescriptorPrimitives.h"

// System Control
namespace kaleidoscope {
namespace hid {
namespace system {

static const byte descriptor[] PROGMEM = {
  //TODO limit to system keys only?
  /*  System Control (Power Down, Sleep, Wakeup, ...) */
  D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,								/* USAGE_PAGE (Generic Desktop) */
  D_USAGE, 0x80,								/* USAGE (System Control) */
  D_COLLECTION, D_APPLICATION, 							/* COLLECTION (Application) */
  D_REPORT_ID, HID_REPORTID_SYSTEMCONTROL,		/* REPORT_ID */
  /* 1 system key */
  D_LOGICAL_MINIMUM, 0x00, 							/* LOGICAL_MINIMUM (0) */
  D_MULTIBYTE(D_LOGICAL_MAXIMUM), 0xff, 0x00, 						/* LOGICAL_MAXIMUM (255) */
  D_USAGE_MINIMUM, 0x00, 							/* USAGE_MINIMUM (Undefined) */
  D_USAGE_MAXIMUM, 0xff, 							/* USAGE_MAXIMUM (System Menu Down) */
  D_REPORT_COUNT, 0x01, 							/* REPORT_COUNT (1) */
  D_REPORT_SIZE, 0x08, 							/* REPORT_SIZE (8) */
  D_INPUT, (D_DATA|D_ARRAY|D_ABSOLUTE), 							/* INPUT (Data,Ary,Abs) */
  D_END_COLLECTION 									/* END_COLLECTION */
};

Report::Report() {
  this->clear();
}

void Report::clear() {
  keycode_ = 0;
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

void Dispatcher::sendReportUnchecked() {
  HID().SendReport(HID_REPORTID_CONSUMERCONTROL, &data_, sizeof(data_));
}

void Dispatcher::sendReport() {
  // If the last report is different than the current report, then we need to send a report.
  // We guard sendReport like this so that calling code doesn't end up spamming the host with empty reports
  // if sendReport is called in a tight loop.

  // if the previous report is the same, return early without a new report.
  if (memcmp(&last_report_.keycodes_, &data_, sizeof(data_)) == 0)
    return;

  sendReportUnchecked();
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
