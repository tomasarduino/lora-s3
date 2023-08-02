/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/queue.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: on
 * - Flow control: off
 * - Event queue: 5
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (18)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM (1)
#define ECHO_UART_BAUD_RATE (9600)
#define ECHO_TASK_STACK_SIZE (1024*2)
#define BUF_SIZE (1024)

static const char *TAG = "UART TEST";
static QueueHandle_t uart_queue;

static void echo_task(void *arg)
{
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    uart_event_t event;
    while (1)
    {
        if(xQueueReceive(uart_queue,(void *)&event,portMAX_DELAY))
        {
            // Receive Buffer
            bzero(data,BUF_SIZE);
            int len = uart_read_bytes(ECHO_UART_PORT_NUM, data,21, pdMS_TO_TICKS(100));
            if (len == 21)
            {
                data[len] = '\0';
                ESP_LOGI(TAG, "Recv str: %s", (char *)data);
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
        }
        bzero(data,BUF_SIZE);        

    }
}

static void init_uart(void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);

    uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);

    uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 5, &uart_queue, 0);

    ESP_LOGI(TAG,"init uart completed!");

    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL,5, NULL);

    ESP_LOGI(TAG,"init uart completed!");

}
void app_main(void)
{
    init_uart();    
}
