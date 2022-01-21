#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include <vector>

namespace esphome {
namespace touchscreen {

#define LOG_TOUCHSCREEN(this) \
  ESP_LOGCONFIG(TAG, "  Width: %d", this->get_display_width()); \
  ESP_LOGCONFIG(TAG, "  Height: %d", this->get_display_height()); \
  ESP_LOGCONFIG(TAG, "  Rotation: %d°", (uint8_t) this->get_rotation() * 90);

struct TouchPoint {
  uint16_t x;
  uint16_t y;
  uint8_t id;
  uint8_t state;
};

class TouchListener {
 public:
  virtual void touch(TouchPoint tp) = 0;
  virtual void release();
};

enum TouchRotation : uint8_t {
  ROTATE_0_DEGREES = 0,
  ROTATE_90_DEGREES,
  ROTATE_180_DEGREES,
  ROTATE_270_DEGREES,
};

class Touchscreen {
 public:
  void set_display_details(uint16_t width, uint16_t height, TouchRotation rotation) {
    this->display_width_ = width;
    this->display_height_ = height;
    this->rotation_ = rotation;
  }

  uint16_t get_display_width() const { return this->display_width_; }
  uint16_t get_display_height() const { return this->display_height_; }
  TouchRotation get_rotation() const { return this->rotation_; }

  Trigger<TouchPoint> *get_touch_trigger() const { return this->touch_trigger_; }

  void register_listener(TouchListener *listener) { this->touch_listeners_.push_back(listener); }

 protected:
  void send_touch_(TouchPoint tp);

  uint16_t display_width_;
  uint16_t display_height_;
  TouchRotation rotation_;
  Trigger<TouchPoint> *touch_trigger_ = new Trigger<TouchPoint>();
  std::vector<TouchListener *> touch_listeners_;
};

}  // namespace touchscreen
}  // namespace esphome
