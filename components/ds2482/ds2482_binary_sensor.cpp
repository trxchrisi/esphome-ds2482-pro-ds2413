#include "ds2482_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ds2482 {

static const char *TAG = "ds2482.binary_sensor";

static bool is_valid_ds2413_status(uint8_t status) {
  const uint8_t low_nibble = status & 0x0F;
  const uint8_t high_nibble = (status >> 4) & 0x0F;
  return high_nibble == static_cast<uint8_t>((~low_nibble) & 0x0F);
}

static bool read_ds2413_status(DS2482Component *parent, uint8_t channel, uint64_t address, uint8_t *status) {
  if (!parent->reset_1w(channel)) {
    return false;
  }

  parent->write_byte_1w(channel, 0x55);  // MATCH ROM
  for (int i = 0; i < 8; i++) {
    const uint8_t byte = static_cast<uint8_t>(address >> (56 - i * 8));
    parent->write_byte_1w(channel, byte);
  }

  parent->write_byte_1w(channel, 0xF5);  // PIO ACCESS READ
  *status = parent->read_byte_1w(channel);
  return true;
}

void DS2482BinarySensor::update() {
  if (!this->parent_->select_channel(this->channel_)) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: channel select failed", this->channel_, this->address_);
    return;
  }

  const uint64_t configured_address = this->address_;
  uint64_t request_address = this->address_locked_ ? this->effective_address_ : configured_address;
  uint8_t status = 0xFF;
  ESP_LOGV(TAG, "[CH:%d] 0x%016llX: DS2413 read start (req=0x%016llX PIO=%s, inverted=%s)", this->channel_, this->address_,
           request_address, this->pio_ == 0 ? "A" : "B", this->inverted_ ? "YES" : "NO");

  if (this->parent_->get_ds2413_cached_status(this->channel_, request_address, &status)) {
    ESP_LOGV(TAG, "[CH:%d] 0x%016llX: cache hit status=0x%02X", this->channel_, this->address_, status);
  } else if (!read_ds2413_status(this->parent_, this->channel_, request_address, &status)) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: no presence pulse", this->channel_, this->address_);
    return;
  }

  bool valid = is_valid_ds2413_status(status);
  ESP_LOGV(TAG, "[CH:%d] 0x%016llX: primary status=0x%02X valid=%s", this->channel_,
           this->address_, status, valid ? "YES" : "NO");

  // Retry once on the same ROM to handle transient line noise.
  if (!valid && !read_ds2413_status(this->parent_, this->channel_, request_address, &status)) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: retry no presence pulse", this->channel_, this->address_);
    return;
  } else if (!valid) {
    valid = is_valid_ds2413_status(status);
    ESP_LOGV(TAG, "[CH:%d] 0x%016llX: retry status=0x%02X valid=%s", this->channel_, this->address_, status,
             valid ? "YES" : "NO");
  }

  if (!valid && !this->address_locked_) {
    // Fallback for installations where scan/config differs by bit 7 in ROM byte 0.
    const uint64_t alt_address = configured_address ^ 0x8000000000000000ULL;
    ESP_LOGV(TAG, "[CH:%d] 0x%016llX: trying fallback address 0x%016llX", this->channel_, this->address_, alt_address);
    if (read_ds2413_status(this->parent_, this->channel_, alt_address, &status)) {
      valid = is_valid_ds2413_status(status);
      ESP_LOGV(TAG, "[CH:%d] 0x%016llX: fallback status=0x%02X valid=%s", this->channel_,
               this->address_, status, valid ? "YES" : "NO");
      if (valid) {
        this->effective_address_ = alt_address;
        this->address_locked_ = true;
        request_address = alt_address;
        ESP_LOGW(TAG, "[CH:%d] 0x%016llX: using fallback ROM 0x%016llX", this->channel_, this->address_, alt_address);
      }
    } else {
      ESP_LOGV(TAG, "[CH:%d] 0x%016llX: fallback address had no presence pulse", this->channel_, this->address_);
    }
  }

  if (!valid) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: invalid DS2413 response 0x%02X", this->channel_, this->address_, status);
    this->parent_->recover_bus();
    return;
  }

  this->parent_->put_ds2413_cached_status(this->channel_, request_address, status);

  // Bit 0 = sensed level PIOA, Bit 1 = sensed level PIOB
  const bool raw_state = ((status >> this->pio_) & 0x01) != 0;
  bool state = raw_state;
  if (this->inverted_) {
    state = !state;
  }
  ESP_LOGV(TAG, "[CH:%d] 0x%016llX: publish state raw=%s final=%s", this->channel_, this->address_,
           raw_state ? "ON" : "OFF", state ? "ON" : "OFF");
  this->publish_state(state);
}

void DS2482BinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "DS2482 DS2413 Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  1-Wire Channel: %u", this->channel_);
  ESP_LOGCONFIG(TAG, "  Address: 0x%016llX", this->address_);
  ESP_LOGCONFIG(TAG, "  Fallback ROM lock: %s", this->address_locked_ ? "ACTIVE" : "OFF");
  ESP_LOGCONFIG(TAG, "  PIO: %s", this->pio_ == 0 ? "A" : "B");
  ESP_LOGCONFIG(TAG, "  Inverted: %s", this->inverted_ ? "YES" : "NO");
}

}  // namespace ds2482
}  // namespace esphome
