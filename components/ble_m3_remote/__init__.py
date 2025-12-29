import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

CODEOWNERS = ["@your_username"]
DEPENDENCIES = ["esp32_ble_tracker", "ble_client"]
AUTO_LOAD = ["binary_sensor"]

CONF_BLE_CLIENT_ID = "ble_client_id"
CONF_BUTTON_UP = "button_up"
CONF_BUTTON_DOWN = "button_down"
CONF_BUTTON_LEFT = "button_left"
CONF_BUTTON_RIGHT = "button_right"
CONF_BUTTON_CENTER = "button_center"
CONF_BUTTON_CAMERA = "button_camera"

ble_m3_remote_ns = cg.esphome_ns.namespace("ble_m3_remote")
BLEM3Remote = ble_m3_remote_ns.class_("BLEM3Remote", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BLEM3Remote),
    cv.Required(CONF_BLE_CLIENT_ID): cv.use_id(cg.esphome_ns.class_("ble_client")("BLEClient")),
    
    cv.Optional(CONF_BUTTON_UP): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_BUTTON_DOWN): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_BUTTON_LEFT): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_BUTTON_RIGHT): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_BUTTON_CENTER): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_BUTTON_CAMERA): binary_sensor.binary_sensor_schema(),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    ble_client = await cg.get_variable(config[CONF_BLE_CLIENT_ID])
    cg.add(var.set_ble_client(ble_client))
    
    if CONF_BUTTON_UP in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_UP])
        cg.add(var.set_button_up_sensor(sens))
    if CONF_BUTTON_DOWN in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_DOWN])
        cg.add(var.set_button_down_sensor(sens))
    if CONF_BUTTON_LEFT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_LEFT])
        cg.add(var.set_button_left_sensor(sens))
    if CONF_BUTTON_RIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_RIGHT])
        cg.add(var.set_button_right_sensor(sens))
    if CONF_BUTTON_CENTER in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_CENTER])
        cg.add(var.set_button_center_sensor(sens))
    if CONF_BUTTON_CAMERA in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BUTTON_CAMERA])
        cg.add(var.set_button_camera_sensor(sens))