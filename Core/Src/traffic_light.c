#include "traffic_light.h"
#include "led.h"
#include "button.h"
#include "main.h"

#define DEFAULT_RED_MS    12000U
#define MIN_TIMEOUT_S     1U
#define MAX_TIMEOUT_S     3600U
#define MS_PER_SECOND     1000U
#define GREEN_BLINK_MS    2000U
#define BLINK_HALF_MS     250U
#define YELLOW_MS         2000U

typedef enum {
    TL_RED,
    TL_GREEN,
    TL_GREEN_BLINK,
    TL_YELLOW
} tl_state_t;

static const char *const state_names[] = {
    [TL_RED]         = "red",
    [TL_GREEN]       = "green",
    [TL_GREEN_BLINK] = "blinking green",
    [TL_YELLOW]      = "yellow",
};

static tl_state_t           state;
static uint32_t             state_start;
static uint32_t             state_duration;
static uint32_t             last_blink_tick;
static bool                 short_red_pending;
static uint32_t             red_ms = DEFAULT_RED_MS;
static traffic_light_mode_t mode   = TRAFFIC_LIGHT_MODE_1;

static uint32_t quarter_of_red_ms(void)
{
    return red_ms / 4U;
}

static uint32_t elapsed(uint32_t now)
{
    return (uint32_t)(now - state_start);
}

static void enter_state(tl_state_t next, uint32_t now)
{
    state       = next;
    state_start = now;
    led_all_off();

    switch (next) {
    case TL_RED:
        led_set(LED_RED, true);
        state_duration    = short_red_pending ? quarter_of_red_ms() : red_ms;
        short_red_pending = false;
        break;
    case TL_GREEN:
        led_set(LED_GREEN, true);
        state_duration = quarter_of_red_ms();
        break;
    case TL_GREEN_BLINK:
        led_set(LED_GREEN, true);
        state_duration  = GREEN_BLINK_MS;
        last_blink_tick = now;
        break;
    case TL_YELLOW:
        led_set(LED_YELLOW, true);
        state_duration = YELLOW_MS;
        break;
    }
}

static void on_button(uint32_t now)
{
    switch (state) {
    case TL_GREEN:
        break;

    case TL_RED: {
        uint32_t spent = elapsed(now);
        if (spent < state_duration) {
            uint32_t left = state_duration - spent;
            if (left > quarter_of_red_ms()) {
                state_duration = spent + quarter_of_red_ms();
            }
        }
        break;
    }

    case TL_GREEN_BLINK:
    case TL_YELLOW:
        short_red_pending = true;
        break;
    }
}

void traffic_light_init(void)
{
    short_red_pending = false;
    enter_state(TL_RED, HAL_GetTick());
}

void traffic_light_update(void)
{
    uint32_t now = HAL_GetTick();

    if (button_pressed_event() && (mode == TRAFFIC_LIGHT_MODE_1)) {
        on_button(now);
    }

    if (state == TL_GREEN_BLINK &&
        (uint32_t)(now - last_blink_tick) >= BLINK_HALF_MS) {
        last_blink_tick = now;
        led_toggle(LED_GREEN);
    }

    if (elapsed(now) >= state_duration) {
        switch (state) {
        case TL_RED:         enter_state(TL_GREEN, now);       break;
        case TL_GREEN:       enter_state(TL_GREEN_BLINK, now); break;
        case TL_GREEN_BLINK: enter_state(TL_YELLOW, now);      break;
        case TL_YELLOW:      enter_state(TL_RED, now);         break;
        }
    }
}

const char *traffic_light_state_name(void)
{
    return state_names[state];
}

void traffic_light_set_mode(traffic_light_mode_t new_mode)
{
    mode = new_mode;
    if (mode == TRAFFIC_LIGHT_MODE_2) {
        short_red_pending = false;
    }
}

traffic_light_mode_t traffic_light_get_mode(void)
{
    return mode;
}

bool traffic_light_set_timeout_s(uint32_t seconds)
{
    if ((seconds < MIN_TIMEOUT_S) || (seconds > MAX_TIMEOUT_S)) {
        return false;
    }
    red_ms = seconds * MS_PER_SECOND;
    return true;
}

uint32_t traffic_light_get_timeout_s(void)
{
    return red_ms / MS_PER_SECOND;
}
