import esphome.codegen as cg
from esphome.components import binary_sensor, sensor, text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
DEPENDENCIES = ["esp32", "wifi"]
CONF_SDA_PIN, CONF_SCL_PIN, CONF_ADDRESS = "sda_pin", "scl_pin", "address"
CONF_ANALOG_RAW_MAX, CONF_DIGITAL_INPUTS = "analog_raw_max", "digital_inputs"
CONF_ANALOG_INPUTS, CONF_DIGITAL_OUTPUTS = "analog_inputs", "digital_outputs"
CONF_EVENT_TYPE, CONF_MODULE, CONF_CHANNEL = "event_type", "module", "channel"
CONF_PRESS_ONLY, CONF_RAW_SENSOR = "press_only", "raw_sensor"
CONF_BAT1_DISPLAY, CONF_BAT2_DISPLAY = "bat1_display", "bat2_display"
CONF_SEGMENT_MODULE = "segment_module"
CONF_SETUP_AP_PASSWORD = "setup_ap_password"

bridge_ns = cg.esphome_ns.namespace("mobiflight_bridge")
MobiFlightBridge = bridge_ns.class_("MobiFlightBridge", cg.Component)

DIGITAL_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.Required(CONF_EVENT_TYPE): cv.hex_uint8_t, cv.Required(CONF_MODULE): cv.uint8_t,
    cv.Required(CONF_CHANNEL): cv.uint8_t, cv.Optional(CONF_PRESS_ONLY, default=False): cv.boolean,
})
ANALOG_SCHEMA = sensor.sensor_schema(unit_of_measurement="%", accuracy_decimals=0).extend({
    cv.Required(CONF_MODULE): cv.uint8_t,
    cv.Required(CONF_RAW_SENSOR): sensor.sensor_schema(accuracy_decimals=0, entity_category="diagnostic"),
})
OUTPUT_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.Required(CONF_EVENT_TYPE): cv.hex_uint8_t, cv.Required(CONF_MODULE): cv.uint8_t,
    cv.Required(CONF_CHANNEL): cv.uint8_t,
})
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MobiFlightBridge),
    cv.Optional(CONF_SDA_PIN, default=2): cv.int_range(min=0, max=48),
    cv.Optional(CONF_SCL_PIN, default=1): cv.int_range(min=0, max=48),
    cv.Optional(CONF_ADDRESS, default=0x42): cv.i2c_address,
    cv.Optional(CONF_ANALOG_RAW_MAX, default=1023): cv.positive_int,
    cv.Optional(CONF_SEGMENT_MODULE, default=0): cv.uint8_t,
    cv.Required(CONF_SETUP_AP_PASSWORD): cv.string_strict,
    cv.Required(CONF_BAT1_DISPLAY): text_sensor.text_sensor_schema(),
    cv.Required(CONF_BAT2_DISPLAY): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_DIGITAL_INPUTS, default=[]): cv.ensure_list(DIGITAL_SCHEMA),
    cv.Optional(CONF_ANALOG_INPUTS, default=[]): cv.ensure_list(ANALOG_SCHEMA),
    cv.Optional(CONF_DIGITAL_OUTPUTS, default=[]): cv.ensure_list(OUTPUT_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_i2c(config[CONF_SDA_PIN], config[CONF_SCL_PIN], config[CONF_ADDRESS]))
    cg.add(var.set_analog_raw_max(config[CONF_ANALOG_RAW_MAX]))
    cg.add(var.set_segment_module(config[CONF_SEGMENT_MODULE]))
    cg.add(var.set_setup_ap_password(config[CONF_SETUP_AP_PASSWORD]))
    bat1 = await text_sensor.new_text_sensor(config[CONF_BAT1_DISPLAY])
    bat2 = await text_sensor.new_text_sensor(config[CONF_BAT2_DISPLAY])
    cg.add(var.set_bat_displays(bat1, bat2))
    for entry in config[CONF_DIGITAL_INPUTS]:
        entity = await binary_sensor.new_binary_sensor(entry)
        cg.add(var.add_digital_input(entry[CONF_EVENT_TYPE], entry[CONF_MODULE], entry[CONF_CHANNEL], entry[CONF_PRESS_ONLY], entity))
    for entry in config[CONF_ANALOG_INPUTS]:
        entity = await sensor.new_sensor(entry)
        raw_entity = await sensor.new_sensor(entry[CONF_RAW_SENSOR])
        cg.add(var.add_analog_input(entry[CONF_MODULE], entity, raw_entity))
    for entry in config[CONF_DIGITAL_OUTPUTS]:
        entity = await binary_sensor.new_binary_sensor(entry)
        cg.add(var.add_digital_output(entry[CONF_EVENT_TYPE], entry[CONF_MODULE], entry[CONF_CHANNEL], entity))
    cg.add_library("Wire", None)
    cg.add_library("m5stack/M5Unified", "0.2.10")
