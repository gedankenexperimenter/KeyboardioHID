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
  data_.keycode = 0;
}

void Report::add(byte keycode) {
  data_.keycode = keycode;
}

void Report::del(byte keycode) {
  clear();
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
  HID().SendReport(HID_REPORTID_SYSTEMCONTROL, &report.data_, sizeof(report.data_));
}

void Dispatcher::sendReport(const Report& report) {
  // If the last report is different than the current report, then we need to send a report.
  // We guard sendReport like this so that calling code doesn't end up spamming the host with empty reports
  // if sendReport is called in a tight loop.

  // if the previous report is the same, return early without a new report.

  // in the case of system control, the report is only one byte, so this could
  // really be simplified a lot. For now, I'm leaving it as similar as possible
  // to the other keyboard reports.
  if (memcmp(&last_report_.keycodes_, &report.data_, sizeof(report.data_)) == 0)
    return;

  if (report.data_.keycode == HID_SYSTEM_WAKE_UP) {
    USBDevice.wakeupHost();
  } else {
    sendReportUnchecked();
  }
  memcpy(&_lastReport, &_report, sizeof(_report));
}

} // namespace system {
} // namespace hid {
} // namespace kaleidoscope {
