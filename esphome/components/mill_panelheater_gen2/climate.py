import esphome.codegen as cg
from esphome.components import climate, uart
import esphome.config_validation as cv

CODEOWNERS = ["@owangen"]

DEPENDENCIES = ["climate", "uart"]

mill_panelheater_gen2_ns = cg.esphome_ns.namespace("mill_panelheater_gen2")
MillHeater = mill_panelheater_gen2_ns.class_(
    "MillPanelHeaterGen2", uart.UARTDevice, climate.Climate, cg.Component
)

CONFIG_SCHEMA = (
    climate.climate_schema(MillHeater)
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
