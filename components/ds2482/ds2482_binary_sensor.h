#pragma once

#include "ds2482.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace ds2482 {

class DS2482BinarySensor : public binary_sensor::BinarySensor, public PollingComponent {
 public:
  void set_parent(DS2482Component *parent) { parent_ = parent; }
  void set_address(uint64_t address) {
    address_ = address;
    effective_address_ = address;
    address_locked_ = false;
  }
  void set_channel(uint8_t channel) { channel_ = channel; }
  void set_pio(uint8_t pio) { pio_ = pio; }
  void set_inverted(bool inverted) { inverted_ = inverted; }

  void update() override;
  void dump_config() override;

 protected:
  DS2482Component *parent_{nullptr};
  uint64_t address_{0};
  uint8_t channel_{0};
  uint8_t pio_{0};  // 0 = PIOA, 1 = PIOB
  bool inverted_{false};
  uint64_t effective_address_{0};
  bool address_locked_{false};
};

}  // namespace ds2482
}  // namespace esphome
