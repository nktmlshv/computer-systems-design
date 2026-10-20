#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TRAFFIC_LIGHT_MODE_1 = 1U,
    TRAFFIC_LIGHT_MODE_2 = 2U
} traffic_light_mode_t;

void                 traffic_light_init(void);
void                 traffic_light_update(void);

const char          *traffic_light_state_name(void);

void                 traffic_light_set_mode(traffic_light_mode_t mode);
traffic_light_mode_t traffic_light_get_mode(void);

bool                 traffic_light_set_timeout_s(uint32_t seconds);
uint32_t             traffic_light_get_timeout_s(void);

#endif
