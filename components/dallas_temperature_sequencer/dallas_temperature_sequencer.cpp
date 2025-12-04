#include "dallas_temperature_sequencer.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dallas_temperature_sequencer {

static const char *const TAG = "dallas_temperature_sequencer";
void DallasTemperatureSequencerComponent::add_sensor(dallas_temp::DallasTemperatureSensor *sensor) {
  if (sensor != nullptr) {
    sensors_.push_back(sensor);
    enabled_.push_back(true);  // default enabled
    // Record the original configured address name (so we can treat placeholders specially)
    if (sensor != nullptr) {
      initial_config_address_names_.push_back(sensor->get_address_name());
    } else {
      initial_config_address_names_.push_back(std::string(""));
    }
    // Initialize assigned name slot
    assigned_address_names_.push_back(std::string(""));
    ESP_LOGD(TAG, "Pushing sensor %s new length %u", sensor->get_address_name().c_str(), (unsigned) sensors_.size());
  }
}

void DallasTemperatureSequencerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Dallas Temperature Sequencer:");
  ESP_LOGCONFIG(TAG, "  OneWire bus: %s", onewire_ != nullptr ? "configured" : "not set");
  ESP_LOGCONFIG(TAG, "  Attached Dallas sensors: %u", (unsigned) sensors_.size());
  for (size_t i = 0; i < sensors_.size(); i++) {
    auto *s = sensors_[i];
    bool en = enabled_[i];
    ESP_LOGCONFIG(TAG, "    [%u] %s (enabled=%s)", (unsigned) i, s != nullptr ? s->get_address_name().c_str() : "<null>", en ? "true" : "false");
  }
}

void DallasTemperatureSequencerComponent::update() {
  if (sensors_.empty())
    return;
  if (recheck) {
    if (onewire_ != nullptr) { 
      for (size_t i = 0; i < sensors_.size(); i++) {
        enabled_[i] = false;
      }
      onewire_->search();
      std::vector<uint64_t> addresses = onewire_->get_devices();

      std::vector<bool> addr_used(addresses.size(), false);

      for (size_t i = 0; i < addresses.size(); i++) {
        std::string address_str = std::string("0x") + format_hex(addresses[i]);
        for (size_t q = 0; q < sensors_.size(); q++) {
          auto *sensor = sensors_[q];
          if (sensor == nullptr) continue;
          const std::string &cfg_addr = (q < initial_config_address_names_.size() ? initial_config_address_names_[q] : sensor->get_address_name());
          const std::string &assigned = (q < assigned_address_names_.size() ? assigned_address_names_[q] : std::string(""));
          if (cfg_addr == address_str || (!assigned.empty() && assigned == address_str)) {
            enabled_[q] = true;
            addr_used[i] = true;
            ESP_LOGV(TAG, "Matched configured sensor %s at index %u", address_str.c_str(), (unsigned) q);
            break;
          }
        }
      }

      const std::string ZERO_ADDR = "0x0000000000000000";
      for (size_t i = 0; i < addresses.size(); i++) {
        if (addr_used[i]) continue;
        std::string address_str = std::string("0x") + format_hex(addresses[i]);

        for (size_t q = 0; q < sensors_.size(); q++) {
          auto *sensor = sensors_[q];
          if (sensor == nullptr) continue;
          const std::string &orig_cfg = (q < initial_config_address_names_.size() ? initial_config_address_names_[q] : sensor->get_address_name());
          if (orig_cfg == ZERO_ADDR) {
            sensor->set_address(addresses[i]);
            if (q < assigned_address_names_.size())
              assigned_address_names_[q] = address_str;
            enabled_[q] = true;
            addr_used[i] = true;
            ESP_LOGI(TAG, "Assigned discovered address %s to placeholder sensor at index %u", address_str.c_str(), (unsigned) q);
            break;
          }
        }
        if (!addr_used[i]) {
          ESP_LOGV(TAG, "Discovered address %s has no available placeholder to assign", address_str.c_str());
        }
      }
    }
    recheck = false;
  } else {
    size_t tried = 0;
    dallas_temp::DallasTemperatureSensor *s = sensors_.at(pos_);
    bool en = enabled_.at(pos_);
    while(s == nullptr || !en) {
      tried++;
      incPos();
      if(tried > sensors_.size()) {
        return;
      }
      ESP_LOGD(TAG, "Sensor index %u not found. Retry %u", (unsigned) pos_, (unsigned) tried);
      s = sensors_.at(pos_);
      en = enabled_.at(pos_);
    }
    ESP_LOGV(TAG, "Updating sensor index %u (%s)", (unsigned) pos_, s->get_address_name().c_str());
    s->update();
    incPos();
  }
}

void DallasTemperatureSequencerComponent::setBase(one_wire::OneWireBus *bus) {
  onewire_ = bus;
  ESP_LOGD(TAG, "OneWire bus set: %p", (void*) bus);
}

void DallasTemperatureSequencerComponent::set_sensor_enabled(dallas_temp::DallasTemperatureSensor *sensor, bool enabled) {
  if (sensor == nullptr) return;
  for (size_t i = 0; i < sensors_.size(); ++i) {
    if (sensors_[i] == sensor) {
      if (i >= enabled_.size()) enabled_.resize(i + 1, true);
      enabled_[i] = enabled;
      break;
    }
  }
}
void DallasTemperatureSequencerComponent::incPos() {
  pos_++;
  if (pos_ >= sensors_.size()) {
    recheck = true;
    pos_ = 0;
  }
}

std::string DallasTemperatureSequencerComponent::get_assigned_address_name_for(dallas_temp::DallasTemperatureSensor *sensor) {
  for (size_t i = 0; i < this->sensors_.size(); ++i) {
    if (this->sensors_[i] == sensor) {
      if (i < this->assigned_address_names_.size() && !this->assigned_address_names_[i].empty())
        return this->assigned_address_names_[i];
      else
        return sensor->get_address_name();
    }
  }
  return std::string("");
}

}  // namespace dallas_temperature_sequencer
}  // namespace esphome
