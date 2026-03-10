# ESPHome DS2482 Pro (Extended)

External component for DS2482-100 and DS2482-800 (I2C to 1-Wire), including:
- Temperature sensors (e.g. DS18B20 family)
- DS2413 binary sensors (PIO A/B)

## Features
- DS2482 ROM search support
- Channel-based scanning for DS2482-800
- Temperature read with CRC check and retry handling
- DS2413 PIO read (`pio: A` / `pio: B`) with optional `inverted`
- Scan button helper to print detected 1-Wire addresses

## Installation

### Option A: Local component (recommended while developing)
```yaml
external_components:
  - source:
      type: local
      path: /config/esphome/components
    components: [ds2482]
```

### Option B: Git source
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/<your-user>/<your-repo>
    components: [ds2482]
```

## Example YAML

```yaml
i2c:
  sda: GPIO4
  scl: GPIO14
  frequency: 100kHz
  scan: true

ds2482:
  id: ow_bus
  address: 0x18

sensor:
  - platform: ds2482
    ds2482_id: ow_bus
    name: "Living Room Temp"
    address: 0x2843C500000000C2
    channel: 6
    update_interval: 60s

binary_sensor:
  - platform: ds2482
    ds2482_id: ow_bus
    name: "Window PIOA"
    address: 0xBA21ADC2C00310F3
    channel: 2
    pio: A
    device_class: window
    inverted: true
    update_interval: 5s

  - platform: ds2482
    ds2482_id: ow_bus
    name: "Window PIOB"
    address: 0xBA21ADC2C00310F3
    channel: 2
    pio: B
    inverted: true
    update_interval: 5s

button:
  - platform: template
    name: "Scan 1-Wire Bus"
    on_press:
      lambda: |-
        id(ow_bus).scan_and_log_devices();
```

## Notes
- Use the scan button to discover addresses and channel mapping.
- If `binary_sensor.ds2482` is not found, verify that `binary_sensor.py` exists in `components/ds2482`.
- After component changes, run a clean rebuild (`Clean Build Files` in ESPHome UI).

## Attribution
This project is based on:
- https://github.com/sermaster2477/esphome-ds2482-pro

Additional changes include DS2413 support, diagnostics, and integration adjustments.

## What Changed (vs. upstream)
- Added DS2413 support as `binary_sensor` platform (`pio: A|B`, optional `inverted`).
- Improved DS2413 diagnostics (raw status, nibble validation, fallback tracing).
- Added scan diagnostics with family code and CRC verification.
- Added practical YAML examples for DS2482 temperature + DS2413 PIO inputs.
- Repository docs expanded (`README.md`).

## License
MIT License.

Original copyright:
- Copyright (c) 2026 Sergey Guzhvin

See [LICENSE](LICENSE) for the full license text.
