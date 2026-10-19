#include "traffic_light.h"
#include "led.h"
#include "button.h"
#include "main.h"

#define RED_MS            12000U
#define RED_SHORT_MS      (RED_MS / 4U)
#define GREEN_MS          (RED_MS / 4U)
#define GREEN_BLINK_MS    2000U
#define BLINK_HALF_MS     250U
#define YELLOW_MS         2000U

typedef enum {
    TL_RED,
    TL_GREEN,
    TL_GREEN_BLINK,
    TL_YELLOW
} tl_state_t;

static tl_state_t state;
static uint32_t   state_start;
static uint32_t   state_duration;
static uint32_t   last_blink_tick;
static bool       short_red_pending;

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
        state_duration    = short_red_pending ? RED_SHORT_MS : RED_MS;
        short_red_pending = false;
        break;
    case TL_GREEN:
        led_set(LED_GREEN, true);
        state_duration = GREEN_MS;
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
            if (left > RED_SHORT_MS) {
                state_duration = spent + RED_SHORT_MS;
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

    if (button_pressed_event()) {
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
