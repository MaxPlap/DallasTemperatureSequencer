# DallasTemperatureSequencer

Short README showing how to use the `DallasTemperatureSequencer`
**Overview**
- **Purpose:** Drive multiple `dallas_temp` sensors in sequence from a single OneWire bus (including DS2484 I2C OneWire bridge). Useful when you want the component to trigger updates avoiding bus jamming and to let placeholder sensors (address = 0x000...0) be auto-assigned from discovered devices on the bus.

**Installation**
- **From GitHub** add to `external_components`
- It also needs this specific [PR](https://github.com/esphome/esphome/pull/12150) that updates the sensor's address
  ```yaml
  external_components:
    - source: github://MaxPlap/DallasTemperatureSequencer
      components: [ dallas_temperature_sequencer ]'
    - source: github://pr#12150
      components: [one_wire]
  ```

- **Local (developer):** place the component directory under `custom/dallas_temperature_sequencer/` in your ESPHome project. ESPHome will pick it up when building.

**Basic sensor configuration**
- Define your `dallas_temp` sensors as usual. If you want the sequencer to drive measurements, set `update_interval: never`. If you want the sequencer to auto-assign an address to a sensor, configure that sensor with a zero address (all zeros):

  ```yaml
  sensor:
    - platform: dallas_temp
      address: 0x7c00000083ae9e28
      id: temp5e
      name: "Temp-5e"
      resolution: 12
      update_interval: never       # DallasTemperatureSequencer will trigger the update

    - platform: dallas_temp
      address: 0x0000000000000000  # placeholder -> allows auto-assignment
      id: generic
      name: "generic"
      resolution: 12
      update_interval: never
  ```

**Sequencer configuration (YAML)**
- Add the `dallas_temperature_sequencer` block and point it at your configured `one_wire` instance and sensors. `interval` controls the time between sensor measurements (per sensor). Example from `birrino.yaml`:

  ```yaml
  dallas_temperature_sequencer:
    onewire: my_ds2484
    sensors:
      - temp5e
      - generic
    interval: .5s
  ```

- Notes:
  - `onewire:` should reference your `one_wire` platform entry (DS2484, gpio-based, etc.).
  - `interval` is the delay between reading successive sensors. Choose a value that matches your sensor responsiveness and the bus speed (e.g. `0.5s` is a good starting point).

**How it works**
- The sequencer repeatedly updates each sensor in the `sensors:` list. It calls `component.update` on each `dallas_temp` sensor and waits for a valid value (the component code handles waiting and rechecks the bus). This lets you use `update_interval: never` on the sensors and have the sequencer control timing.
- Sensors declared with a zero address (`0x0000000000000000`) act as placeholders. On boot or during rechecks the sequencer scans the bus and will assign discovered addresses to placeholder sensors (in-memory mapping). This keeps placeholders interchangeable and avoids touching protected internals of the core Dallas sensor objects.

**Accessing assigned address/name in display lambdas**
- The `dallas_temp` sensor exposes `get_address_name()` which returns the configured or assigned address string. You can show this in a display lambda, for example:

  ```cpp
  it.printf(0, 16, id(font1), "%.1f %s", id(generic).state, id(generic)->get_address_name().c_str());
  ```

**Recommended settings**
- Set `update_interval: never` on `dallas_temp` sensors driven by the sequencer.
- Use `interval` >= 0.5s to be conservative with bus timing and to avoid interfering with other I2C/OneWire traffic.
- If using a DS2484 bridge, ensure your `i2c` frequency and DS2484 settings are compatible (example uses `frequency: 200kHz`).

**Troubleshooting**
- If placeholder sensors are not getting assigned:
  - Confirm the OneWire bus is functional and that the DS2484 or GPIO bus is configured correctly.
  - Check logs for the sequencer assignment messages (the component logs when it scans and assigns addresses).
  - If you need persistent assignments across reboots, consider adding text_sensors or implementing storage of assigned names in preferences/NVS (not implemented by default).

- If `get_address_name()` returns an empty string, ensure the sequencer scanned the bus and/or the sensor has been updated at least once by the sequencer.

**Example**

```yaml
one_wire:
  - platform: ds2484
    id: my_ds2484
    i2c_id: bus_i2c
    address: 0x18
    active_pullup: true

dallas_temperature_sequencer:
  onewire: my_ds2484
  sensors:
    - temp5e
    - generic
  interval: .5s
```
