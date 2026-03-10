#include "ds2482_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ds2482 {

static const char *TAG = "ds2482.binary_sensor";

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

  uint8_t status = 0xFF;
  ESP_LOGD(TAG, "[CH:%d] 0x%016llX: DS2413 read start (PIO=%s, inverted=%s)", this->channel_, this->address_,
           this->pio_ == 0 ? "A" : "B", this->inverted_ ? "YES" : "NO");
  if (!read_ds2413_status(this->parent_, this->channel_, this->address_, &status)) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: no presence pulse", this->channel_, this->address_);
    return;
  }

  // DS2413 returns data in the low nibble, high nibble is inverted low nibble.
  const uint8_t low_nibble = status & 0x0F;
  const uint8_t high_nibble = (status >> 4) & 0x0F;
  bool valid = high_nibble == static_cast<uint8_t>((~low_nibble) & 0x0F);
  ESP_LOGD(TAG, "[CH:%d] 0x%016llX: primary status=0x%02X low=0x%X high=0x%X valid=%s", this->channel_,
           this->address_, status, low_nibble, high_nibble, valid ? "YES" : "NO");

  if (!valid) {
    // Fallback: some installations report ROM with bit 7 of byte 0 flipped.
    // Retry once with that bit toggled before failing hard.
    const uint64_t alt_address = this->address_ ^ 0x8000000000000000ULL;
    ESP_LOGD(TAG, "[CH:%d] 0x%016llX: trying fallback address 0x%016llX", this->channel_, this->address_, alt_address);
    if (read_ds2413_status(this->parent_, this->channel_, alt_address, &status)) {
      const uint8_t alt_low_nibble = status & 0x0F;
      const uint8_t alt_high_nibble = (status >> 4) & 0x0F;
      valid = alt_high_nibble == static_cast<uint8_t>((~alt_low_nibble) & 0x0F);
      ESP_LOGD(TAG, "[CH:%d] 0x%016llX: fallback status=0x%02X low=0x%X high=0x%X valid=%s", this->channel_,
               this->address_, status, alt_low_nibble, alt_high_nibble, valid ? "YES" : "NO");
      if (valid) {
        ESP_LOGW(TAG, "[CH:%d] 0x%016llX: address fallback active, check scanned ROM", this->channel_, this->address_);
      }
    } else {
      ESP_LOGD(TAG, "[CH:%d] 0x%016llX: fallback address had no presence pulse", this->channel_, this->address_);
    }
  }

  if (!valid) {
    ESP_LOGW(TAG, "[CH:%d] 0x%016llX: invalid DS2413 response 0x%02X", this->channel_, this->address_, status);
    this->parent_->recover_bus();
    return;
  }

  // Bit 0 = sensed level PIOA, Bit 1 = sensed level PIOB
  const bool raw_state = ((status >> this->pio_) & 0x01) != 0;
  bool state = raw_state;
  if (this->inverted_) {
    state = !state;
  }
  ESP_LOGD(TAG, "[CH:%d] 0x%016llX: publish state raw=%s final=%s", this->channel_, this->address_,
           raw_state ? "ON" : "OFF", state ? "ON" : "OFF");
  this->publish_state(state);
}

void DS2482BinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "DS2482 DS2413 Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  1-Wire Channel: %u", this->channel_);
  ESP_LOGCONFIG(TAG, "  Address: 0x%016llX", this->address_);
  ESP_LOGCONFIG(TAG, "  PIO: %s", this->pio_ == 0 ? "A" : "B");
  ESP_LOGCONFIG(TAG, "  Inverted: %s", this->inverted_ ? "YES" : "NO");
}

}  // namespace ds2482
}  // namespace esphome
