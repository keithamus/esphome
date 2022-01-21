#include "touchscreen.h"

namespace esphome {
namespace touchscreen {

static const char *const TAG = "touchscreen";

void Touchscreen::send_touch_(TouchPoint tp) {
  ESP_LOGV(TAG, "Touch %d: (x=%d, y=%d)", i, tp.x, tp.y);
  this->touch_trigger_->trigger(tp);
  for (auto *listener : this->touch_listeners_)
    listener->touch(tp);
}

}  // namespace touchscreen
}  // namespace esphome
