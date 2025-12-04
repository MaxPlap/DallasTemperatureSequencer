import esphome.config_validation as cv
import esphome.codegen as cg

from esphome.const import CONF_ID
dallas_temp_ns = cg.esphome_ns.namespace("dallas_temp")
DallasTemperatureSensor = dallas_temp_ns.class_("DallasTemperatureSensor")
one_wire_ns = cg.esphome_ns.namespace("one_wire")
OneWireBus = one_wire_ns.class_("OneWireBus")

MULTI_CONF = True
DEPENDENCIES = ["one_wire"]
AUTO_LOAD = ["dallas_temp"]
CONF_SENSORS = "sensors"
CONF_INTERVAL = "interval"
CONF_ONEWIRE = "onewire"

dts_ns = cg.esphome_ns.namespace("dallas_temperature_sequencer")
DallasTemperatureSequencerComponent = dts_ns.class_(
    "DallasTemperatureSequencerComponent", cg.Component
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DallasTemperatureSequencerComponent),
            cv.Required(CONF_ONEWIRE): cv.use_id(OneWireBus),
            cv.Required(CONF_SENSORS): cv.ensure_list(cv.use_id(DallasTemperatureSensor)),
            cv.Optional(CONF_INTERVAL): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    bus = await cg.get_variable(config[CONF_ONEWIRE])
    cg.add(var.setBase(bus))
    for s in config[CONF_SENSORS]:
        ds = await cg.get_variable(s)
        cg.add(var.add_sensor(ds))
    if (interval := config.get(CONF_INTERVAL)) is not None:
        cg.add(var.set_update_interval(interval))
