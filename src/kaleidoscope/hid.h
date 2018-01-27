// -*- c++ -*-

// This file only contains the API that other modules should use
namespace kaleidoscope {
namespace hid {


namespace keyboard {

class Report {
 public:
  Report();
  void clear();
  void add(byte keycode);
  void del(byte keycode);
  byte modifiers();
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
  byte ledState();
  byte lastModifierState();
};

} // namespace keyboard {


namespace consumer {

class Report {
 public:
  Report();
  void clear();
  void add(Key keycode);
  void del(Key keycode);
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
};

} // namespace consumer {


namespace system {

class Report {
 public:
  Report();
  void clear();
  void add(byte keycode);
  void del(); // same as clear()?
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
};

} // namespace system {


namespace mouse {

constexpr byte BUTTON_LEFT   = B00000001;
constexpr byte BUTTON_MIDDLE = B00000010;
constexpr byte BUTTON_RIGHT  = B00000100;

class Report {
  void clear();
  void add(byte button); // press
  void del(byte button); // release
  byte buttons(); // one byte with all button states?
  void move(int8_t x, int8_t y, int8_t v_wheel, int8_t h_wheel);
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
  byte lastButtonState(); // ?
};

} // namespace mouse {


namespace mouse_abs {

constexpr byte BUTTON_LEFT   = B00000001;
constexpr byte BUTTON_MIDDLE = B00000010;
constexpr byte BUTTON_RIGHT  = B00000100;

class Report {
  void clear();
  void add(byte button); // press
  void del(byte button); // release
  byte buttons(); // one byte with all button states?
  void move(int8_t x, int8_t y, int8_t wheel);
  void moveTo(uint16_t x, uint16_t y, int8_t wheel);
};

class Dispatcher {
 public:
  Dispatcher();
  void init();
  void sendReport(const Report &report);
  byte lastButtonState(); // ?
};

} // namespace mouse_abs {


} // namespace hid {
} // namespace kaleidoscope {
