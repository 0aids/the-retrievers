#include "driver/uart.h"
#include "gps_state.h"

#define GPS_UART_NUM UART_NUM_1
#define GPS_UART_BAUD 9600

#define GPS_TASK_PRIORITY \
    5  // gps only is at 1 time per second so no need give priority
#define GPS_TASK_STACK_SIZE 4096

#define GPS_TX_PIN 27
#define GPS_RX_PIN 25
#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 0  // cause we arent sending anything to gps

void gps_task() {
    // we finna do the gps stuff type shit
}

void gps_init() {
    uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(GPS_UART_NUM, &uart_config);

    uart_set_pin(GPS_UART_NUM, GPS_TX_PIN, GPS_RX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    uart_driver_install(GPS_UART_NUM, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 0, NULL,
                        0);

    gps_state_init();
}

void gps_start() {
    xTaskCreate(gps_task, "gps_task", GPS_TASK_STACK_SIZE, NULL,
                GPS_TASK_PRIORITY, NULL);
}