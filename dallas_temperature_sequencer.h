#pragma once

#include "esphome/core/component.h"
#include "esphome/components/dallas_temp/dallas_temp.h"
#include "esphome/components/one_wire/one_wire.h"
#include <vector>
namespace esphome
{
    namespace dallas_temperature_sequencer
    {

        class DallasTemperatureSequencerComponent : public PollingComponent
        {
        private:
            std::vector<dallas_temp::DallasTemperatureSensor *> sensors_;
            std::vector<bool> enabled_;
            std::vector<std::string> assigned_address_names_;
            std::vector<std::string> initial_config_address_names_;
            size_t pos_ = 0;
            bool recheck = false;
            one_wire::OneWireBus *onewire_ = nullptr;
            void incPos();
        public:
            DallasTemperatureSequencerComponent() : PollingComponent(1000) {}

            void update() override;
            void dump_config() override;
            void add_sensor(dallas_temp::DallasTemperatureSensor *sensor);
            void setBase(one_wire::OneWireBus *bus);
            void set_sensor_enabled(dallas_temp::DallasTemperatureSensor *sensor, bool enabled);
            // Optional: expose assigned address name for a sensor
            std::string get_assigned_address_name_for(dallas_temp::DallasTemperatureSensor *sensor);
        };
    }
}
