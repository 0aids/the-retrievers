#pragma once

#include <stdint.h>

#include "driver/gpio.h"
#include "sm.h"

typedef enum {
    button_id_landing,
    button_id_prelaunch,
    button_id_ldr,
    button_id__COUNT
} button_id_e;

typedef struct {
    gpio_num_t pin;
    gpio_int_type_t intr_type;
    psatFSM_eventType_e event;
} button_config_t;

void button_init(void);
void button_enable(button_id_e id);
void button_disable(button_id_e id);
void button_delete(button_id_e id);
