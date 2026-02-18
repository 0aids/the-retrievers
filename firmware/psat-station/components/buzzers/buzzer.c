#include "buzzer.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "pin_config.h"
#include "shared_state.h"
#include "sm.h"
#include "errors.h"

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
    if(gpio_config(&io_conf) != ESP_OK){
        psatErr_postError(psatErr_buzzer_gpioConfig_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    };

    if(gpio_set_level(CFG_BUZZER_PIN_d, 0)!= ESP_OK){
        psatErr_postError(psatErr_buzzer_gpioInitLevel_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    }

    esp_timer_create_args_t timer_args = {.callback = &beepTimerCb,
                                          .name     = "buzzer_timer"};

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &beepTimer_s));
    ESP_LOGI(TAG, "Buzzers initialised");
}

void buzzer_deinit(void)
{
    if (beepTimer_s != NULL)
    {
        esp_timer_stop(beepTimer_s);
        esp_timer_delete(beepTimer_s);
        beepTimer_s = NULL;
    }

    buzzerActive_s = false;
    if(gpio_set_level(CFG_BUZZER_PIN_d, 0) != ESP_OK){
        psatErr_postError(psatErr_buzzer_gpioDeinitLevel_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    }
    if(gpio_reset_pin(CFG_BUZZER_PIN_d) != ESP_OK) {
        psatErr_postError(psatErr_buzzer_gpioReset_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    }

    ESP_LOGI(TAG, "Buzzers deintied");
}

void buzzer_turnOn(void)
{
    if(gpio_set_level(CFG_BUZZER_PIN_d, 1) != ESP_OK) {
        psatErr_postError(psatErr_buzzer_turnOn_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    }
    buzzerActive_s = true;
}

void buzzer_turnOff(void)
{
    if(gpio_set_level(CFG_BUZZER_PIN_d, 0) != ESP_OK) {
        psatErr_postError(psatErr_buzzer_turnOff_failed, psatFSM_component_buzzers, psatFSM_getCurrentState());
        return;
    }
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
