import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ADDRESS, CONF_CHANNEL, CONF_ID, CONF_INVERTED

from . import CONF_DS2482_ID, DS2482Component, ds2482_ns

CONF_PIO = "pio"

DS2482BinarySensor = ds2482_ns.class_(
    "DS2482BinarySensor", binary_sensor.BinarySensor, cg.PollingComponent
)

PIO_OPTIONS = {
    "A": 0,
    "B": 1,
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(DS2482BinarySensor).extend(
    {
        cv.GenerateID(CONF_DS2482_ID): cv.use_id(DS2482Component),
        cv.Required(CONF_ADDRESS): cv.hex_uint64_t,
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=7),
        cv.Optional(CONF_PIO, default="A"): cv.enum(PIO_OPTIONS, upper=True),
        cv.Optional(CONF_INVERTED, default=False): cv.boolean,
    }
).extend(cv.polling_component_schema("1s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)

    parent = await cg.get_variable(config[CONF_DS2482_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_pio(config[CONF_PIO]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
