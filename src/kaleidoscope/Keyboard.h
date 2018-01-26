// -*- c++ -*-

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


class Report {
  friend class Dispatcher;
 public:
  Report();
  void clear();
  constexpr void add(byte keycode) { setKeycode(keycode, true); }
  constexpr void del(byte keycode) { setKeycode(keycode, false); }
  constexpr byte modifiers() { return data_.modifiers; }
  // Do I want a function to get the state of any given keycode?
  bool isActive(byte keycode);
 private:
  struct {
    byte modifiers;
    byte keycodes[KEY_BYTES];
  } data_;
  void setKeycode(byte keycode, bool val);
};


class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
  byte ledState();
  byte lastModifierState();
 private:
  Report last_report_;
  void sendReportUnchecked(const Report &report);
};


} // namespace keyboard {
} // namespace hid {
} // namespace kaleidoscope {
