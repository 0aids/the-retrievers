#include "buttons.h"

#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

#define DEBOUNCE_MS 50

static button_config_t buttonConfig_c[button_id__COUNT] = {
    [button_id_landing]   = {.intr_type = GPIO_INTR_NEGEDGE,
                             .event =
                                 psatFSM_eventType_landingConfirmed,       .pin = CFG_LANDING_BUTTON_PIN_d  },
    [button_id_prelaunch] = {.intr_type = GPIO_INTR_NEGEDGE,
                             .event =
                                 psatFSM_eventType_prelaunchComplete,      .pin = CFG_PRELAUNCH_BUTTON_PIN_d},
    [button_id_ldr]       = {.intr_type = GPIO_INTR_NEGEDGE,
                             .event = psatFSM_eventType_deploymentPending,
                             .pin   = CFG_LIGHT1_PIN_d                                                      },
};

static volatile TickType_t lastPressTick_s[button_id__COUNT];

static void IRAM_ATTR      button_isr(void* arg)
{
    button_id_e id  = (button_id_e)arg;
    TickType_t  now = xTaskGetTickCountFromISR();

    if ((now - lastPressTick_s[id]) < pdMS_TO_TICKS(DEBOUNCE_MS))
    {
        return;
    }

    lastPressTick_s[id] = now;

    psatFSM_event_t event = {.type   = buttonConfig_c[id].event,
                             .global = false};
    psatFSM_postEventISR(&event);
}

void button_init(void)
{
    gpio_install_isr_service(0);

    for (int i = 0; i < button_id__COUNT; i++)
    {
        gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << buttonConfig_c[i].pin,
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = buttonConfig_c[i].intr_type,
        };

        gpio_config(&cfg);
        lastPressTick_s[i] = 0;
    }
}

void button_enable(button_id_e id)
{
    gpio_isr_handler_add(buttonConfig_c[id].pin, button_isr,
                         (void*)id);
    gpio_intr_enable(buttonConfig_c[id].pin);
}

void button_disable(button_id_e id)
{
    gpio_intr_disable(buttonConfig_c[id].pin);
    gpio_isr_handler_remove(buttonConfig_c[id].pin);
}

void button_delete(button_id_e id)
{
    button_disable(id);
    gpio_reset_pin(buttonConfig_c[id].pin);
}
