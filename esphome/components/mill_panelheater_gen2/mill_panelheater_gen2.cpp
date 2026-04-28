#include "mill_panelheater_gen2.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mill_panelheater_gen2 {

static const char *const TAG = "millpanelheatergen2.climate";

void MillPanelHeaterGen2::setup() {
  this->traits_.set_visual_target_temperature_step(1);
  this->traits_.set_visual_current_temperature_step(1);
  this->traits_.set_visual_min_temperature(5);
  this->traits_.set_visual_max_temperature(35);
  this->traits_.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  this->traits_.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT,
  });
  ESP_LOGI(TAG, "MillPanelHeaterGen2 initialization...");
}

void MillPanelHeaterGen2::dump_config() {
  ESP_LOGCONFIG(TAG, "MillPanelHeaterGen2:");
  LOG_CLIMATE("", "MillPanelHeaterGen2 Climate", this);
  this->check_uart_settings(9600);
}

void MillPanelHeaterGen2::loop() {
  this->recv_with_start_end_markers_();

  if (this->new_data_) {
    this->new_data_ = false;
    if ((uint8_t) this->received_chars_[COMMAND_TYPE_POS] == 0xC9) {
      if (this->received_chars_[TARGET_TEMP_POS] != 0) {
        this->target_temperature = this->received_chars_[TARGET_TEMP_POS];
      }
      if (this->received_chars_[CURRENT_TEMP_POS] != 0) {
        this->current_temperature = this->received_chars_[CURRENT_TEMP_POS];
      }
      if (this->received_chars_[MODE_POS] == 0x00) {
        this->mode = climate::CLIMATE_MODE_OFF;
        this->action = climate::CLIMATE_ACTION_OFF;
      } else if (this->received_chars_[MODE_POS] == 0x01) {
        this->mode = climate::CLIMATE_MODE_HEAT;
      }
      this->action = (this->received_chars_[ACTION_POS] == 0x00) ? climate::CLIMATE_ACTION_IDLE
                                                                  : climate::CLIMATE_ACTION_HEATING;
      this->publish_state();
    }
  }
}

void MillPanelHeaterGen2::recv_with_start_end_markers_() {
  while (this->available() > 0) {
    uint8_t rc = this->read();
    if (this->recv_in_progress_) {
      if (rc != END_MARKER && rc != LINE_END_MARKER) {
        if (this->recv_index_ < BUFFER_SIZE) {
          this->received_chars_[this->recv_index_++] = (char) rc;
        }
      } else {
        this->recv_in_progress_ = false;
        this->recv_index_ = 0;
        this->new_data_ = true;
      }
    } else if (rc == START_MARKER) {
      this->recv_in_progress_ = true;
    }
  }
}

climate::ClimateTraits MillPanelHeaterGen2::traits() { return this->traits_; }

void MillPanelHeaterGen2::control(const climate::ClimateCall &call) {
  ESP_LOGD(TAG, "Climate change requested");

  if (call.get_mode().has_value()) {
    switch (call.get_mode().value()) {
      case climate::CLIMATE_MODE_OFF:
        this->send_command_(this->power_command_, sizeof(this->power_command_), 0x00);
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->send_command_(this->power_command_, sizeof(this->power_command_), 0x01);
        break;
      default:
        break;
    }
    this->mode = call.get_mode().value();
    this->publish_state();
  }

  if (call.get_target_temperature().has_value()) {
    int temp = (int) call.get_target_temperature().value();
    this->send_command_(this->temperature_command_, sizeof(this->temperature_command_), temp);
    this->target_temperature = temp;
    this->publish_state();
  }
}

void MillPanelHeaterGen2::send_command_(uint8_t *command_array, int len, int command) {
  ESP_LOGD(TAG, "Sending serial command");
  if (command_array[4] == 0x46) {  // Temperature command
    command_array[7] = command;
  }
  if (command_array[4] == 0x47) {  // Power on/off command
    command_array[5] = command;
    command_array[len] = 0x00;
  }
  uint8_t crc = this->checksum_(command_array, len + 1);
  this->write_byte(START_MARKER);
  for (int i = 0; i < len + 1; i++) {
    this->write_byte(command_array[i]);
  }
  this->write_byte(crc);
  this->write_byte(END_MARKER);
}

uint8_t MillPanelHeaterGen2::checksum_(uint8_t *buf, int len) {
  uint8_t chk = 0;
  for (; len != 0; len--) {
    chk += *buf++;
  }
  return chk;
}

}  // namespace mill_panelheater_gen2
}  // namespace esphome
