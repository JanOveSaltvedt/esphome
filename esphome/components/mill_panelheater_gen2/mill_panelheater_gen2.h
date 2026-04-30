#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mill_panelheater_gen2 {

class MillPanelHeaterGen2 : public Component, public climate::Climate, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void control(const climate::ClimateCall &call) override;
  void dump_config() override;

  void set_wattage(float wattage) { this->wattage_ = wattage; }
  void set_power_sensor(sensor::Sensor *s) { this->power_sensor_ = s; }

 protected:
  climate::ClimateTraits traits() override;

  static constexpr size_t BUFFER_SIZE = 15;

  static constexpr size_t COMMAND_TYPE_POS = 4;
  static constexpr size_t TARGET_TEMP_POS = 6;
  static constexpr size_t CURRENT_TEMP_POS = 7;
  static constexpr size_t MODE_POS = 9;
  static constexpr size_t ACTION_POS = 11;

  static constexpr uint8_t START_MARKER = 0x5A;
  static constexpr uint8_t END_MARKER = 0x5B;
  static constexpr uint8_t LINE_END_MARKER = 0x0A;

  char received_chars_[BUFFER_SIZE]{};
  bool new_data_{false};
  bool recv_in_progress_{false};
  uint8_t recv_index_{0};

  climate::ClimateTraits traits_;

  float wattage_{0.0f};
  sensor::Sensor *power_sensor_{nullptr};

  uint8_t power_command_[12]{0x00, 0x10, 0x06, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t temperature_command_[12]{0x00, 0x10, 0x22, 0x00, 0x46, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00};

 private:
  void recv_with_start_end_markers_();
  void send_command_(uint8_t *command_array, int len, int command);
  uint8_t checksum_(uint8_t *buf, int len);
};

}  // namespace mill_panelheater_gen2
}  // namespace esphome
