// -*- c++ -*-
#if 0
/*
Copyright (c) 2014-2015 NicoHood
Copyright (c) 2015-2018 Keyboard.io, Inc

See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <Arduino.h>
#include <HID.h>

#include "kaleidoglyph/hid/mouse.h"
#include "kaleidoglyph/hid/descriptor_constants.h"

namespace kaleidoglyph {
namespace hid {
namespace mouse {

static const PROGMEM byte mouse_descriptor[] = {
  /*  Mouse relative */
  D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,           // USAGE_PAGE (Generic Desktop)
  D_USAGE, D_USAGE_MOUSE,                         //  USAGE (Mouse)
  D_COLLECTION, D_APPLICATION,                    //   COLLECTION (Application)
  D_REPORT_ID, HID_REPORTID_MOUSE,                //    REPORT_ID (Mouse)

  /* 8 Buttons */
  D_USAGE_PAGE, D_PAGE_BUTTON,                    //    USAGE_PAGE (Button)
  D_USAGE_MINIMUM, 0x01,                          //     USAGE_MINIMUM (Button 1)
  D_USAGE_MAXIMUM, 0x08,                          //     USAGE_MAXIMUM (Button 8)
  D_LOGICAL_MINIMUM, 0x00,                        //     LOGICAL_MINIMUM (0)
  D_LOGICAL_MAXIMUM, 0x01,                        //     LOGICAL_MAXIMUM (1)
  D_REPORT_COUNT, 0x08,                           //     REPORT_COUNT (8)
  D_REPORT_SIZE, 0x01,                            //     REPORT_SIZE (1)
  D_INPUT, (D_DATA|D_VARIABLE|D_ABSOLUTE),        //     INPUT (Data,Var,Abs)

  /* X, Y, Wheel */
  D_USAGE_PAGE, D_PAGE_GENERIC_DESKTOP,           //    USAGE_PAGE (Generic Desktop)
  D_USAGE, 0x30,                                  //     USAGE (X)
  D_USAGE, 0x31,                                  //     USAGE (Y)
  D_USAGE, 0x38,                                  //     USAGE (Wheel)
  D_LOGICAL_MINIMUM, 0x81,                        //     LOGICAL_MINIMUM (-127)
  D_LOGICAL_MAXIMUM, 0x7f,                        //     LOGICAL_MAXIMUM (127)
  D_REPORT_SIZE, 0x08,                            //     REPORT_SIZE (8)
  D_REPORT_COUNT, 0x03,                           //     REPORT_COUNT (3)
  D_INPUT, (D_DATA|D_VARIABLE|D_RELATIVE),        //     INPUT (Data,Var,Rel)

  /* Horizontal wheel */
  D_USAGE_PAGE, D_PAGE_CONSUMER,                  //    USAGE_PAGE (Consumer)
  D_PAGE_ORDINAL, 0x38, 0x02,                     //     PAGE (AC Pan)
  D_LOGICAL_MINIMUM, 0x81,                        //     LOGICAL_MINIMUM (-127)
  D_LOGICAL_MAXIMUM, 0x7f,                        //     LOGICAL_MAXIMUM (127)
  D_REPORT_SIZE, 0x08,                            //     REPORT_SIZE (8)
  D_REPORT_COUNT, 0x01,                           //     REPORT_COUNT (1)
  D_INPUT, (D_DATA|D_VARIABLE|D_RELATIVE),        //     INPUT (Data,Var,Rel)

  /* End */
  D_END_COLLECTION                                // END_COLLECTION
};

void Dispatcher::init() {
  static HIDSubDescriptor node(mouse_descriptor, sizeof(mouse_descriptor));
  HID().AppendDescriptor(&node);

  end();
}

void Dispatcher::end() {
  releaseAll();
  sendReport();
}

void Dispatcher::click(byte buttons) {
  press(b);
  sendReport();
  release(b);
}

void Dispatcher::move(int8_t x, int8_t y,
                      int8_t v_wheel, int8_t h_wheel) {
  report.x = x;
  report.y = y;
  report.v_wheel = vWheel;
  report.h_wheel = hWheel;
}

void Dispatcher::releaseAll(void) {
  memset(&report, 0, sizeof(report));
}

void Dispatcher::press(uint8_t b) {
  report.buttons |= b;
}

void Dispatcher::release(uint8_t b) {
  report.buttons &= ~b;
}

bool Dispatcher::isPressed(uint8_t b) {
  if ((b & report.buttons) > 0)
    return true;
  return false;
}

void Dispatcher::sendReportUnchecked(void) {
  HID().SendReport(HID_REPORTID_MOUSE, &report, sizeof(report));
}

void Dispatcher::sendReport(void) {
  // If the last report is different than the current report, then we need to send a report.
  // We guard sendReport like this so that calling code doesn't end up spamming the host with empty reports
  // if sendReport is called in a tight loop.

  // if the two reports are the same, check if they're empty, and return early
  // without a report if they are.
  static HID_MouseReport_Data_t emptyReport;
  if (memcmp(&lastReport, &report, sizeof(report)) == 0 &&
      memcmp(&report, &emptyReport, sizeof(report)) == 0)
    return;

  sendReportUnchecked();
  memcpy(&lastReport, &report, sizeof(report));
}

} // namespace mouse {
} // namespace hid {
} // namespace kaleidoscope {
#endif
