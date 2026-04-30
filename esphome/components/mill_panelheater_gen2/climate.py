import esphome.codegen as cg
from esphome.components import climate, sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_POWER,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    UNIT_WATT,
)

CODEOWNERS = ["@owangen"]

DEPENDENCIES = ["climate", "uart"]
AUTO_LOAD = ["sensor"]

CONF_WATTAGE = "wattage"

mill_panelheater_gen2_ns = cg.esphome_ns.namespace("mill_panelheater_gen2")
MillHeater = mill_panelheater_gen2_ns.class_(
    "MillPanelHeaterGen2", uart.UARTDevice, climate.Climate, cg.Component
)

CONFIG_SCHEMA = (
    climate.climate_schema(MillHeater)
    .extend(
        {
            cv.Optional(CONF_WATTAGE): cv.positive_float,
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


def _validate_wattage(config):
    if CONF_POWER in config and CONF_WATTAGE not in config:
        raise cv.Invalid("'wattage' is required when 'power' sensor is configured")
    return config


FINAL_VALIDATE_SCHEMA = _validate_wattage


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_WATTAGE in config:
        cg.add(var.set_wattage(config[CONF_WATTAGE]))

    if power_config := config.get(CONF_POWER):
        sens = await sensor.new_sensor(power_config)
        cg.add(var.set_power_sensor(sens))
