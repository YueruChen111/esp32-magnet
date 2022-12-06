#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "driver/timer.h"
#define COM_TXD (GPIO_NUM_21)
#define COM_RXD (GPIO_NUM_20)
#define COM_RTS (UART_PIN_NO_CHANGE)
#define COM_CTS (UART_PIN_NO_CHANGE)
#define COM_UART_PORT_NUM      0
#define DOUT_PIN 4
#define COM_UART_BAUD_RATE     115200
#define COM_TASK_STACK_SIZE    4096
#define BUF_SIZE 1024
#define PROMPT_MSG "> "
#define COMMAND_FUNCTION(name) \
        esp_err_t command_func_##name(char* rx_buffer, \
                       int rx_len, \
                       char* tx_buffer, \
                       int tx_len)
#define MATCH_CMD(x, cmd) (strncasecmp(x, cmd, strlen(cmd)) == 0)

static const char* TAG = "magnet";
static bool state = 0;

static void configure_uart(void) {    //配置UART
    /* Configure parameters of an UART driver,
    * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = COM_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(COM_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(COM_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(COM_UART_PORT_NUM, COM_TXD, COM_RXD, COM_RTS, COM_CTS));
}

static int uart_read_line(uint8_t* buf, size_t len) {    //读入命令并回应
    bzero(buf, len);
    size_t cnt = 0;
    size_t recv_len = 0;
    uint8_t byte_buf = 0;
    while (1) {
        byte_buf = 0;
        recv_len = uart_read_bytes(COM_UART_PORT_NUM, &byte_buf, 1, 20 / portTICK_RATE_MS);  //读入数据储存到byte_buf
        if (recv_len == 1) {
            if (byte_buf == '\r' || byte_buf == '\n') {
                return cnt + 1;
            }
            else if (byte_buf >= 0x20 && byte_buf < 0x7f) {
                buf[cnt++] = byte_buf;
            }
            else if (byte_buf == 0x8) {
                if (cnt > 0) {
                    cnt--;
                    buf[cnt] = 0;
                    uart_write_bytes(COM_UART_PORT_NUM, &byte_buf, 1);
                }
                continue;
            }

            if ((cnt + 1) >= len) {
                return -1;
            }
            // TODO: Responce to backspace (0x7f)

            uart_write_bytes(COM_UART_PORT_NUM, &byte_buf, 1);
            // ESP_LOGI(TAG, "Recv str: %s, end: %d", (char*)buf, buf[cnt - 1]);
            // ESP_LOGI(TAG, "%d", byte_buf);
        }
    }
}

COMMAND_FUNCTION(magnet_restart) {
    esp_restart();
    return ESP_OK;
}

COMMAND_FUNCTION(magnet_enable) {
  //snprintf(tx_buffer, tx_len, "Magnet Enabled\n");
  state=true;
  digitalWrite(DOUT_PIN,state);
  return ESP_OK;
}

COMMAND_FUNCTION(magnet_is_enabled) {
    if(state==true){
      snprintf(tx_buffer, tx_len, "Magnet_is_Enabled\n");
    }
    return ESP_OK;
}

COMMAND_FUNCTION(magnet_disable) {
  //snprintf(tx_buffer, tx_len, "Magnet Disabled\n");
  state=false;
  digitalWrite(DOUT_PIN,state);
  return ESP_OK;
}

COMMAND_FUNCTION(magnet_is_disabled) {
    if(state==false){
      snprintf(tx_buffer, tx_len, "Magnet_is_Disabled\n");
    }
    return ESP_OK;
}

COMMAND_FUNCTION(magnet_status) {
    if(state==true){
      snprintf(tx_buffer, tx_len, "Enabled\n");
    }
    else{
      snprintf(tx_buffer, tx_len, "Disabled\n");
    }
    return ESP_OK;
}

typedef struct command_reg_t
{
    char *name;
    esp_err_t(*func)(char*, int, char*, int);
} command_reg_t;

static command_reg_t s_registration[] = {
        {.name = "restart", .func = command_func_magnet_restart},
        {.name = "enable", .func = command_func_magnet_enable},
        {.name = "is_enabled", .func = command_func_magnet_is_enabled},
        {.name = "disable", .func = command_func_magnet_disable},
        {.name = "is_disabled", .func = command_func_magnet_is_disabled},
        {.name = "get_status", .func = command_func_magnet_status},
};

static struct command_reg_t* parse_command(char* command, int len) {
    //ESP_LOGI(TAG, "Got command %s from controller", command);
    for (int idx = 0; idx < sizeof(s_registration) / sizeof(command_reg_t); ++idx) {
        if (MATCH_CMD(command, s_registration[idx].name)) {
            return &s_registration[idx];
        }
    }
    return NULL;
}

esp_err_t execute_command(char* rx_buffer, char* tx_buffer, size_t rx_len, size_t tx_len) {
    command_reg_t* cmd = parse_command(rx_buffer, rx_len);
    esp_err_t ret = ESP_FAIL;
    bzero(tx_buffer, sizeof(char) * tx_len);

    if (cmd == NULL) {
        /** Output error **/
        ESP_LOGE(TAG, "Got invalid command : %s", rx_buffer);

        /** Fill tx_buffer with 'ERROR\n' **/
        strcpy(tx_buffer, "COMMAND NOT FOUND\n\n");

        /** Return False **/
        return ESP_FAIL;
    }
    else {
        /** Output info **/

        /** Fill tx_buffer with '\0' **/
        /** Fill tx buffer with command related context **/
        ret = cmd->func(rx_buffer, rx_len, tx_buffer, tx_len);

        /** Command did not modify the buffer **/
        // if (strlen(tx_buffer) == 0) {
        //     snprintf(tx_buffer, tx_len, "%s", (ret == ESP_OK) ? " OK\n\n" : " ERROR\n\n");
        // }

        /** Return False **/
        return ret;
    }
};

static void interact_task(void* arg) {
    int recv_len;
    ESP_LOGI(TAG, "Interacting");
    uint8_t* rx_buffer = (uint8_t*)malloc(BUF_SIZE);
    uint8_t* tx_buffer = (uint8_t*)malloc(BUF_SIZE);
    do {
        uart_write_bytes(COM_UART_PORT_NUM, PROMPT_MSG, strlen(PROMPT_MSG));
        recv_len = uart_read_line(rx_buffer, BUF_SIZE);
        if (recv_len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        }
        else {
            rx_buffer[recv_len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", recv_len, rx_buffer);
            execute_command((char*)rx_buffer, (char*)tx_buffer, BUF_SIZE, BUF_SIZE);
            uart_write_bytes(COM_UART_PORT_NUM, (const char*)tx_buffer, strlen((char*)tx_buffer));
        }
    } while (1);
}

void setup() {
  pinMode(DOUT_PIN, OUTPUT);
  //digitalWrite(DOUT_PIN,1);
  // Serial.begin(115200);
  configure_uart();
  ESP_LOGI(TAG, "OK");
   xTaskCreate(interact_task, "uart_echo_task", COM_TASK_STACK_SIZE, NULL, 10, NULL);
}

void loop() {

}
