#include "button.h"
#include "board_config.h"

#define BUTTON_DEBOUNCE_MS  30U

static bool     raw_prev;
static bool     stable_state;
static bool     press_event;
static uint32_t last_change_tick;

static bool read_raw(void)
{
    return gpio_read_pin(BUTTON_PORT, BUTTON_PIN) == BUTTON_ACTIVE_LEVEL;
}

void button_init(void)
{
    gpio_config_t config = {
        .mode     = PIN_MODE_INPUT,
        .otype    = PIN_OTYPE_PUSH_PULL,
        .speed    = PIN_SPEED_LOW,
        .pull     = BUTTON_PULL,
        .alt_func = 0U,
        .level    = PIN_LEVEL_KEEP
    };
    gpio_init(BUTTON_PORT, BUTTON_PIN, &config);

    raw_prev         = read_raw();
    stable_state     = raw_prev;
    press_event      = false;
    last_change_tick = HAL_GetTick();
}

void button_update(void)
{
    uint32_t now = HAL_GetTick();
    bool raw = read_raw();

    if (raw != raw_prev) {
        raw_prev = raw;
        last_change_tick = now;
    } else if (raw != stable_state &&
               (uint32_t)(now - last_change_tick) >= BUTTON_DEBOUNCE_MS) {
        stable_state = raw;
        if (stable_state) {
            press_event = true;
        }
    }
}

bool button_pressed_event(void)
{
    bool event = press_event;
    press_event = false;
    return event;
}
