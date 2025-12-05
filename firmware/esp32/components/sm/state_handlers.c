#include "state_handlers.h"

#include "audio.h"
#include "audio2.h"
#include "deployment.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "gps_driver_esp32.h"
#include "mock_buttons.h"
#include "sm.h"
#include "timers.h"

#define MECHANICAL_DEPLOY_DELAY_SECONDS 30

fsm_state_t state_prelaunch(const fsm_event_t* event) {
    static const char* TAG = "FSM-PRELAUNCH";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_START_PRELAUNCH:
            ESP_LOGI(TAG, "Initilising timer, gps, audio and sensor");

            gps_init();
            audio_init();
            audio2_init();
            timers_start_10s();
            deployment_init_isr();

            // mock buttons
            prelaunch_init_isr();
            landing_init_isr();

            // fsm_event_t start_event = {.type = EVENT_PRELAUNCH_COMPLETE};
            // fsm_post_event(&start_event);

            return STATE_PRELAUNCH;

        case EVENT_PRELAUNCH_COMPLETE:
            ESP_LOGI(TAG, "prelaunch complete moving into ascent...");
            stop_prelaunch_button();
            return STATE_ASCENT;

        default:
            return STATE_PRELAUNCH;
    }
}

fsm_state_t state_ascent(const fsm_event_t* event) {
    static const char* TAG = "FSM-ASCENT";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_TIMER_10S:
            ESP_LOGI(TAG, "Its been 10s, lets ping with lora rn");
            // lora send pong data here
            return STATE_ASCENT;

        case EVENT_DEPLOYMENT_CONFIRMED:
            ESP_LOGI(TAG,
                     "Deployment confirmed, starting: gps, 5s & 1s timers, "
                     "stopping: 10s timer & deployment sensor");

            timers_stop_10s();
            // start the camera
            gps_start();
            timers_start_5s();
            timers_start_1s();
            stop_deployment_button();
            timers_start_mechanical(MECHANICAL_DEPLOY_DELAY_SECONDS);

            return STATE_DEPLOYED;

        default:
            return STATE_ASCENT;
    }
}

fsm_state_t state_deployed(const fsm_event_t* event) {
    static const char* TAG = "FSM-DEPLOYED";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_TIMER_5S:
            // we are currently waiting for camera to turn on, if the 5 second
            // timer fires before camera turns on we move on anyways
            // this is just so if camera fails or smth we dont stop the mission
            ESP_LOGI(TAG, "camera didnt turn on but we move on anyways");
            return STATE_DECENT;
        case EVENT_CAMERA_ON:  // if the camera is on before the timer is up, we
                               // move on early
            return STATE_DECENT;
        default:
            return STATE_DEPLOYED;
    }
}

fsm_state_t state_descent(const fsm_event_t* event) {
    static const char* TAG = "FSM-DESCENT";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_TIMER_1S:
            ESP_LOGI(TAG, "its been 1 second, camera go snap");
            // take a photo here
            return STATE_DECENT;
        case EVENT_TIMER_5S:
            ESP_LOGI(TAG, "its been 5 seconds, sending gps data to ground");

            gps_state_t gps_snapshot;
            gps_get_snapshot(&gps_snapshot);

            if (gps_snapshot.fix_info_valid) {
                log_gps_data(&gps_snapshot);
                // lora gonna send gps here

            } else {
                ESP_LOGI(TAG, "FIX NOT VALID - NOT CONNECTED");
            }

            return STATE_DECENT;
        case EVENT_LANDING_CONFIRMED:
            ESP_LOGI(TAG, "landing has been detected, stopping 1s timer");
            timers_stop_1s();
            stop_landing_button();
            // stop the camera
            return STATE_LANDING;
        default:
            return STATE_DECENT;
    }
}

fsm_state_t state_landing(const fsm_event_t* event) {
    static const char* TAG = "FSM-LANDING";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_TIMER_5S:
            // same like before but a stop version
            ESP_LOGI(TAG, "camera didnt turn off but we move on anyways");
            return STATE_RECOVERY;

        case EVENT_CAMERA_STOP:
            return STATE_RECOVERY;
        default:
            return STATE_LANDING;
    }
}

fsm_state_t state_recovery(const fsm_event_t* event) {
    static const char* TAG = "FSM-RECOVERY";
    ESP_LOGI(TAG, "Event %d", event->type);

    switch (event->type) {
        case EVENT_TIMER_5S:
            ESP_LOGI(TAG, "its been 5 seconds, sending gps data to ground");
            gps_state_t gps_snapshot;
            gps_get_snapshot(&gps_snapshot);

            if (gps_snapshot.fix_info_valid) {
                log_gps_data(&gps_snapshot);
                // lora gonna send gps here

            } else {
                ESP_LOGI(TAG, "FIX NOT VALID - NOT CONNECTED");
            }

            // alternatively we could stop gps and just lora send smth like hey
            // guys im alive or smth idk
            ESP_LOGI(
                TAG,
                "its been 5 seconds, yo ground im still alive find me pls");
            return STATE_RECOVERY;
        case EVENT_LORA_COMMAND:
            ESP_LOGI(TAG, "recovery got lora cmd: %s", event->data.str);
            fsm_event_t lora_event = {0};
            bool got_cmd = false;

            if (strcmp(event->data.str, "AUDIO_ON") == 0) {
                lora_event.type = EVENT_AUDIO_ON;
                got_cmd = true;
            }
            if (strcmp(event->data.str, "AUDIO_OFF") == 0) {
                lora_event.type = EVENT_AUDIO_OFF;
                got_cmd = true;
            }
            if (strcmp(event->data.str, "AUDIO_BEEP") == 0) {
                lora_event.type = EVENT_AUDIO_BEEP;
                got_cmd = true;
            }

            if (got_cmd) {
                fsm_post_event(&lora_event);
            }

            return STATE_RECOVERY;
        case EVENT_AUDIO_ON:
            ESP_LOGI(TAG, "AUDIO ON");
            audio_on();
            return STATE_RECOVERY;

        case EVENT_AUDIO_OFF:
            ESP_LOGI(TAG, "AUDIO OFF");
            audio_off();
            return STATE_RECOVERY;

        case EVENT_AUDIO_FUN:
            ESP_LOGI(TAG, "AUDIO :)");
            audio2_song();
            return STATE_RECOVERY;

        case EVENT_AUDIO_BEEP:
            ESP_LOGI(TAG, "AUDIO BEEP");
            audio_beep(2500);
            audio2_beep(2500);
            return STATE_RECOVERY;
        default:
            return STATE_RECOVERY;
    }
}

fsm_state_t state_error(const fsm_event_t* event) {
    static const char* TAG = "FSM-ERROR";
    ESP_LOGI(TAG, "Event %d, data in it: %s", event->type, event->data.str);

    // this is not really implemented
    switch (event->type) {
        default:
            return STATE_ERROR;
    }
}
