#include "buzzer.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "pin_config.h"

static const char*        TAG = "Buzzers";

static esp_timer_handle_t beepTimer_s;
static bool               buzzerActive_s = false;

static void               beepTimerCb(void* arg)
{
    gpio_set_level(CFG_BUZZER_PIN_d, 0);
    buzzerActive_s = false;
}

void buzzer_init(void)
{
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << CFG_BUZZER_PIN_d,
                             .mode         = GPIO_MODE_OUTPUT,
                             .pull_up_en   = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type    = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(CFG_BUZZER_PIN_d, 0);

    esp_timer_create_args_t timer_args = {.callback = &beepTimerCb,
                                          .name     = "buzzer_timer"};

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &beepTimer_s));
    ESP_LOGI(TAG, "Buzzers initialised");
}

void buzzer_turnOn(void)
{
    gpio_set_level(CFG_BUZZER_PIN_d, 1);
    buzzerActive_s = true;
}

void buzzer_turnOff(void)
{
    gpio_set_level(CFG_BUZZER_PIN_d, 0);
    buzzerActive_s = false;
}

void buzzer_beep(uint32_t durationMs)
{
    if (durationMs == 0)
    {
        return;
    }

    if (buzzerActive_s)
    {
        esp_timer_stop(beepTimer_s);
    }

    buzzer_turnOn();
    esp_timer_start_once(beepTimer_s, durationMs * 1000);
}
