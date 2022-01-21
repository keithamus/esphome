#include "lilygo_t5_47_touchscreen.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lilygo_t5_47_touchscreen {

static const char *const TAG = "lilygo_t5_47_touchscreen";

static const uint8_t POWER_REGISTER = 0xD6;
static const uint8_t TOUCH_REGISTER = 0xD0;

static const uint8_t WAKEUP_CMD[1] = {0x06};
static const uint8_t CLEAR_FLAGS[2] = {0x00, 0xAB};
static const uint8_t READ_TOUCH[1] = {0x07};

#define ERROR_CHECK(err) \
  if (err != i2c::ERROR_OK) { \
    ESP_LOGE(TAG, "Failed to communicate!"); \
    this->status_set_warning(); \
    return; \
  }

void Store::gpio_intr(Store *store) { store->touch = true; }

void LilygoT547Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Lilygo T5 4.7 Touchscreen...");
  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->interrupt_pin_->setup();

  this->store_.pin = this->interrupt_pin_->to_isr();
  this->interrupt_pin_->attach_interrupt(Store::gpio_intr, &this->store_, gpio::INTERRUPT_FALLING_EDGE);

  if (this->write(nullptr, 0) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to communicate!");
    this->interrupt_pin_->detach_interrupt();
    this->mark_failed();
    return;
  }

  this->write_register(POWER_REGISTER, WAKEUP_CMD, 1);
}

void LilygoT547Touchscreen::loop() {
  if (!this->store_.touch)
    return;
  this->store_.touch = false;

  uint8_t point = 0;
  uint8_t buffer[40] = {0};
  uint32_t sumL = 0, sumH = 0;

  i2c::ErrorCode err;
  err = this->write_register(TOUCH_REGISTER, {0x00}, 1);
  ERROR_CHECK(err);

  err = this->read(buffer, 7);
  ERROR_CHECK(err);

  if (buffer[0] == 0xAB) {
    this->write_register(TOUCH_REGISTER, CLEAR_FLAGS, 2);
    return;
  }

  point = buffer[5] & 0xF;

  if (point == 1) {
    err = this->write_register(TOUCH_REGISTER, READ_TOUCH, 1);
    ERROR_CHECK(err);
    err = this->read(&buffer[5], 2);
    ERROR_CHECK(err);

    sumL = buffer[5] << 8 | buffer[6];
  } else if (point > 1) {
    err = this->write_register(TOUCH_REGISTER, READ_TOUCH, 1);
    ERROR_CHECK(err);
    err = this->read(&buffer[5], 5 * (point - 1) + 3);
    ERROR_CHECK(err);

    sumL = buffer[5 * point + 1] << 8 | buffer[5 * point + 2];
  }

  this->write_register(TOUCH_REGISTER, CLEAR_FLAGS, 2);

  for (int i = 0; i < 5 * point; i++)
    sumH += buffer[i];

  if (sumL != sumH)
    point = 0;

  if (point) {
    uint8_t offset;
    for (int i = 0; i < point; i++) {
      if (i == 0)
        offset = 0;
      else
        offset = 4;

      TouchPoint tp;

      tp.id = (buffer[i * 5 + offset] >> 4) & 0x0F;
      tp.state = buffer[i * 5 + offset] & 0x0F;
      if (tp.state == 0x06)
        tp.state = 0x07;

      tp.y = (uint16_t) ((buffer[i * 5 + 1 + offset] << 4) | ((buffer[i * 5 + 3 + offset] >> 4) & 0x0F));
      tp.x = (uint16_t) ((buffer[i * 5 + 2 + offset] << 4) | (buffer[i * 5 + 3 + offset] & 0x0F));

      this->defer([this, tp]() { this->send_touch_(tp); });
    }
  } else {
    TouchPoint tp;
    tp.id = (buffer[0] >> 4) & 0x0F;
    tp.state = 0x06;
    tp.y = (uint16_t) ((buffer[0 * 5 + 1] << 4) | ((buffer[0 * 5 + 3] >> 4) & 0x0F));
    tp.x = (uint16_t) ((buffer[0 * 5 + 2] << 4) | (buffer[0 * 5 + 3] & 0x0F));

    this->defer([this, tp]() { this->send_touch_(tp); });
  }

  this->status_clear_warning();
}

void LilygoT547Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "Lilygo T5 47 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
}

}  // namespace lilygo_t5_47_touchscreen
}  // namespace esphome
