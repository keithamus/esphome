import esphome.config_validation as cv
import esphome.codegen as cg

from esphome import automation
from esphome.const import CONF_HEIGHT, CONF_ON_TOUCH, CONF_ROTATION, CONF_WIDTH

CODEOWNERS = ["@jesserockz"]

touchscreen_ns = cg.esphome_ns.namespace("touchscreen")

Touchscreen = touchscreen_ns.class_("Touchscreen")
TouchRotation = touchscreen_ns.enum("TouchRotation")
TouchPoint = touchscreen_ns.struct("TouchPoint")
TouchListener = touchscreen_ns.class_("TouchListener")

CONF_TOUCHSCREEN_ID = "touchscreen_id"

ROTATIONS = {
    0: TouchRotation.ROTATE_0_DEGREES,
    90: TouchRotation.ROTATE_90_DEGREES,
    180: TouchRotation.ROTATE_180_DEGREES,
    270: TouchRotation.ROTATE_270_DEGREES,
}


def validate_rotation(value):
    value = cv.string(value)
    if value.endswith("°"):
        value = value[:-1]
    return cv.enum(ROTATIONS, int=True)(value)


def touchscreen_schema(height: int, width: int, rotation: int = 0):
    return cv.Schema(
        {
            cv.Optional(CONF_HEIGHT, default=height): cv.positive_int,
            cv.Optional(CONF_WIDTH, default=width): cv.positive_int,
            cv.Optional(CONF_ROTATION, default=rotation): validate_rotation,
            cv.Optional(CONF_ON_TOUCH): automation.validate_automation(single=True),
        }
    )


async def register_touchscreen(var, config):
    cg.add(
        var.set_display_details(
            config[CONF_WIDTH],
            config[CONF_HEIGHT],
            config[CONF_ROTATION],
        )
    )

    if CONF_ON_TOUCH in config:
        await automation.build_automation(
            var.get_touch_trigger(),
            [(TouchPoint, "touch")],
            config[CONF_ON_TOUCH],
        )
