#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#include <stdint.h>
#include "main.h"

static inline uint32_t critical_enter(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void critical_exit(uint32_t primask)
{
    __set_PRIMASK(primask);
}

#endif
