/*********************************************************************
 * Description: Graduation Project
 * Project: To handle communication server - client 
 * Author: Tran Doan Manh
 ********************************************************************/

    /* ------------------------- All libraries using for Web --------------------------*/
#include <esp_wifi.h>
#include "esp_event.h"
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>
#include "Handle_ServerClient.h"
#include "NVS_Flash_Add/NVS_Flash_Add.h"

    /* ------------------------- All libraries using for communicating between ESP32 and ZC --------------------------*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "Handle_ServerClient/Handle_ServerClient.h"

    /* ------------------------- Private macro using for Web -------------------------- */
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN  (64)

    /* ------------------------- Global variable using for Web --------------------------*/
static const char *TAG_HTTP = "HTTP_SERVER_CLIENT";
httpd_handle_t server = NULL;

    // Variable to get registered information in NVS Flash
extern char register_data[20]; 

    // To convert between indexlogin and indexcontrol                                 
static uint8_t index_indexcontrol = 0;   

    // To embed indexlogin.html into Flash
extern const uint8_t indexlogin_html_start[] asm("_binary_indexlogin_html_start");
extern const uint8_t indexlogin_html_end[] asm("_binary_indexlogin_html_end");

    // To embed indexcontrol.html into Flash
extern const uint8_t indexcontrol_html_start[] asm("_binary_indexcontrol_html_start");
extern const uint8_t indexcontrol_html_end[] asm("_binary_indexcontrol_html_end");

    /* ------------------------- Private macro using for USART2 -------------------------- */
#define EX_UART_NUM UART_NUM_2
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
   
       /* ------------------------- Global variable using for USART2 --------------------------*/
static const char *TAG_USART2 = "USART2_RX_EVENTS";
static QueueHandle_t uart2_queue;
int temp_value = 0;
int hum_value = 0;
int light_value = 0;
int status_Light = 0;
int status_NetworkUp = 0;
int status_NetworkDown = 0;
int control_Mode = 0;

    /************************************************** GET Method ************************************************/

    /*------------------------------ Client get indexlogin.html or indexcontrol.html ------------------------------*/

static esp_err_t webServer_handler(httpd_req_t *req)
{
    if(index_indexcontrol == 0)
    {
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, (const char *)indexlogin_html_start, indexlogin_html_end - indexlogin_html_start);
    }
    else if (index_indexcontrol == 1)
    {
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, (const char *)indexcontrol_html_start, indexcontrol_html_end - indexcontrol_html_start);
        index_indexcontrol = 0;
    }

    return ESP_OK;
}

static const httpd_uri_t webServer = {
    .uri       = "/webServer",
    .method    = HTTP_GET,
    .handler   = webServer_handler,
    /* Let's pass response string in user context to demonstrate it's usage */
    .user_ctx  = NULL
};

    /*------------------------------ Client get sensor data to update ------------------------------*/

static esp_err_t get_sensordata_handler(httpd_req_t *req)
{
        /* Send the sensor data to WebServer using JSON string */
    static char sensorData[50];
    memset(sensorData, '\0', sizeof(sensorData));
    sprintf(sensorData, 
            "{\"temperature\": \"%d\",\"humidity\": \"%d\",\"light\": \"%d\"}", 
            temp_value, hum_value, light_value);
    httpd_resp_send(req, sensorData, strlen(sensorData));
    
    return ESP_OK;
}

static const httpd_uri_t get_sensordata = {
    .uri       = "/get_sensordata",
    .method    = HTTP_GET,
    .handler   = get_sensordata_handler,
    /* Let's pass response string in user context to demonstrate it's usage */
    .user_ctx  = NULL
};

    /*------------------------------ Client get status of stair light to update ------------------------------*/

static esp_err_t get_modeAndStatusData_handler(httpd_req_t *req)
{
        /* Send the sensor data to WebServer using JSON string */
    static char modeAndStatus[100];
    memset(modeAndStatus, '\0', sizeof(modeAndStatus));
    sprintf(modeAndStatus,
            "{\"statusLight\": \"%d\",\"statusNetworkUp\": \"%d\",\"statusNetworkDown\": \"%d\",\"controlMode\": \"%d\"}",
            status_Light, status_NetworkUp, status_NetworkDown, control_Mode);
    httpd_resp_send(req, modeAndStatus, strlen(modeAndStatus));
    return ESP_OK;
}

static const httpd_uri_t get_modeAndStatusData = {
    .uri       = "/get_modeAndStatusData",
    .method    = HTTP_GET,
    .handler   = get_modeAndStatusData_handler,
    /* Let's pass response string in user context to demonstrate it's usage */
    .user_ctx  = NULL
};

    /********************************************* POST Method ******************************************/
   
    /*------------------------------ Client post the login data to Server ------------------------------*/

static esp_err_t post_datalogin_handler(httpd_req_t *req)
{
        // Array to store the login data is sent from Client
    char buf[20];

        // Read the login data is sent from Client
    httpd_req_recv(req, buf, req->content_len);

        // Read the registered data is stored in NVS Flash
    readString_NVSFlash();

        // Check between login data and registration data to allow or not allow user to access indexcontrol.html
    if(strcmp(buf, register_data) == 0)
    {
        index_indexcontrol = 1;
    }
                 
        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_datalogin = {
    .uri       = "/login",
    .method    = HTTP_POST,
    .handler   = post_datalogin_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post the registration data to Server ------------------------------*/

static esp_err_t post_dataregister_handler(httpd_req_t *req)
{
        // Array to store the registration data is sent from Client
    char buf[20];

        // Read the login data is sent from Client and display
    httpd_req_recv(req, buf, req->content_len);
    printf("%s\n", buf);

        // Store the registration data into NVS Flash
    writeString_NVSFlash(buf);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_dataregister = {
    .uri       = "/register",
    .method    = HTTP_POST,
    .handler   = post_dataregister_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post creating Network to Server ------------------------------*/

static esp_err_t post_createNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("CREATE", "[The length of creating messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_createNetwork = {
    .uri       = "/createNetwork",
    .method    = HTTP_POST,
    .handler   = post_createNetwork_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post openning Network to Server ------------------------------*/

static esp_err_t post_openNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("OPEN", "[The length of openning messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_openNetwork = {
    .uri       = "/openNetwork",
    .method    = HTTP_POST,
    .handler   = post_openNetwork_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post stopping openning Network to Server ------------------------------*/

static esp_err_t post_stopOpenningNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("STOP", "[The length of stopping messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_stopOpenningNetwork = {
    .uri       = "/stopOpenningNetwork",
    .method    = HTTP_POST,
    .handler   = post_stopOpenningNetwork_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post leaving Network to Server ------------------------------*/

static esp_err_t post_allSW_LeaveNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("ALL LEAVE", "[The length of all leaving messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_allSW_LeaveNetwork= {
    .uri       = "/allSW_LeaveNetwork",
    .method    = HTTP_POST,
    .handler   = post_allSW_LeaveNetwork_handler,
    .user_ctx  = NULL
};

static esp_err_t post_upperSW_LeaveNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("UPPER LEAVE", "[The length of upper leaving messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_upperSW_LeaveNetwork= {
    .uri       = "/upperSW_LeaveNetwork",
    .method    = HTTP_POST,
    .handler   = post_upperSW_LeaveNetwork_handler,
    .user_ctx  = NULL
};

static esp_err_t post_belowSW_LeaveNetwork_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("BELOW LEAVE", "[The length of below leaving messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_belowSW_LeaveNetwork= {
    .uri       = "/belowSW_LeaveNetwork",
    .method    = HTTP_POST,
    .handler   = post_belowSW_LeaveNetwork_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post the control mode to Server ------------------------------*/

static esp_err_t post_autoMode_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("AUTO MODE", "[The length of auto mode messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_autoMode= {
    .uri       = "/autoMode",
    .method    = HTTP_POST,
    .handler   = post_autoMode_handler,
    .user_ctx  = NULL
};

static esp_err_t post_manualMode_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("MANUAL MODE", "[The length of auto mode messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_manualMode= {
    .uri       = "/manualMode",
    .method    = HTTP_POST,
    .handler   = post_manualMode_handler,
    .user_ctx  = NULL
};

    /*------------------------------ Client post the light control data to Server ------------------------------*/

static esp_err_t post_upperSW_TurnOn_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("UPPER TURN ON", "[The length of upper turning on messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_upperSW_TurnOn= {
    .uri       = "/upperSW_TurnOn",
    .method    = HTTP_POST,
    .handler   = post_upperSW_TurnOn_handler,
    .user_ctx  = NULL
};

static esp_err_t post_upperSW_TurnOff_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("UPPER TURN OFF", "[The length of upper turning off messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_upperSW_TurnOff= {
    .uri       = "/upperSW_TurnOff",
    .method    = HTTP_POST,
    .handler   = post_upperSW_TurnOff_handler,
    .user_ctx  = NULL
};

static esp_err_t post_belowSW_TurnOn_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("BELOW TURN ON", "[The length of below turning on messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_belowSW_TurnOn= {
    .uri       = "/belowSW_TurnOn",
    .method    = HTTP_POST,
    .handler   = post_belowSW_TurnOn_handler,
    .user_ctx  = NULL
};

static esp_err_t post_belowSW_TurnOff_handler(httpd_req_t *req)
{
        // Array to store the control data is sent from Client
    char buf[20];

        // Read the control data is sent from Client
    httpd_req_recv(req, buf, req->content_len);
    printf("Control data: %s\n", buf);

        // Send the control data for ZC device
    int length_sent = uart_write_bytes(EX_UART_NUM, (const char*) buf, strlen(buf));
    ESP_LOGI("BELOW TURN OFF", "[The length of below turning off messenger]: %d", length_sent);

        // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_belowSW_TurnOff= {
    .uri       = "/belowSW_TurnOff",
    .method    = HTTP_POST,
    .handler   = post_belowSW_TurnOff_handler,
    .user_ctx  = NULL
};

/*
 * @func start_webserver
 * @brief To start WebServer
 * @param void
 * @retval void
 */
void start_webserver(void)
{
        // To configure httpd
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

        // Start the httpd server
    ESP_LOGI(TAG_HTTP, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
            // Set URI handlers
        ESP_LOGI(TAG_HTTP, "Registering URI handlers");
        httpd_register_uri_handler(server, &webServer);
        httpd_register_uri_handler(server, &post_datalogin);
        httpd_register_uri_handler(server, &post_dataregister);
        httpd_register_uri_handler(server, &post_createNetwork);
        httpd_register_uri_handler(server, &post_openNetwork);
        httpd_register_uri_handler(server, &post_stopOpenningNetwork);
        httpd_register_uri_handler(server, &post_allSW_LeaveNetwork);
        httpd_register_uri_handler(server, &post_upperSW_LeaveNetwork);
        httpd_register_uri_handler(server, &post_belowSW_LeaveNetwork);
        httpd_register_uri_handler(server, &post_autoMode);
        httpd_register_uri_handler(server, &post_manualMode);
        httpd_register_uri_handler(server, &post_upperSW_TurnOn);
        httpd_register_uri_handler(server, &post_upperSW_TurnOff);
        httpd_register_uri_handler(server, &post_belowSW_TurnOn);
        httpd_register_uri_handler(server, &post_belowSW_TurnOff);
        httpd_register_uri_handler(server, &get_sensordata);
        httpd_register_uri_handler(server, &get_modeAndStatusData);
    }
    else{
        ESP_LOGI(TAG_HTTP, "Error starting server!");
    } 
}

/*
 * @func stop_webserver
 * @brief To stop WebServer
 * @param void
 * @retval void
 */
void stop_webserver(void)
{
        // Stop the httpd server
    httpd_stop(server);
}

    /********************************************* All functions for USART2 ******************************************/

/*
 * @func USART2_RX_Events_task
 * @brief To process all RX events
 * @param
 * @retval void
 */
void USART2_RX_Events_task(void *pvParameters)
{
    // To declare variable to process RX_USART events
    uart_event_t event;

    // Allocate memory to store data
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);

    // Process RX_USART events
    for (;;) 
    {
        //Waiting for UART event
        if (xQueueReceive(uart2_queue, (void *)&event, (TickType_t)portMAX_DELAY)) 
        {
            bzero(dtmp, RD_BUF_SIZE);
            switch (event.type) 
            {
            // Process event of UART receving data. We'd better handler data event fast, there would be much more data events 
            // than other types of events. If we take too much time on data event, the queue might be full
            case UART_DATA:
                ESP_LOGI(TAG_USART2, "[UART DATA]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                switch(dtmp[0])
                {
                    case STAIR_LIGHT_CODE:
                        status_Light = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[LIGHT_STATUS]: 0x%x", status_Light);
                        break;

                    case TEMP_VALUE_CODE:
                        temp_value = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[TEMP]: %d", *(dtmp+1));
                        break;

                    case HUM_VALUE_CODE:
                        hum_value = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[HUM]: %d", hum_value);
                        break;

                    case LIGHT_INTENSITY_CODE:
                        light_value = *(dtmp+1) | (*(dtmp+2) << 8);
                        ESP_LOGI(TAG_USART2, "[LIGHT]: %d", light_value);
                        break;

                    case NETWORK_UP_CODE:
                        status_NetworkUp = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[NETWORK_UP]: 0x%x", status_NetworkUp);
                        break;

                    case NETWORK_DOWN_CODE:
                        status_NetworkDown = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[NETWORK_DOWN]: 0x%x", status_NetworkDown);
                        break;

                    case CONTROL_MODE_CODE:
                        control_Mode = *(dtmp+1);
                        ESP_LOGI(TAG_USART2, "[CONTROL_MODE]: 0x%x", control_Mode);
                        break;

                    default:
                        break;
                }
                break;

            // Process event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG_USART2, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart2_queue);
                break;

            // Process event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG_USART2, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart2_queue);
                break;

            // Process event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG_USART2, "uart rx break");
                break;

            // Process event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG_USART2, "uart parity error");
                break;

            // Process event of UART frame erroridf
            case UART_FRAME_ERR:
                ESP_LOGI(TAG_USART2, "uart frame error");
                break;

            // Process others
            default:
                ESP_LOGI(TAG_USART2, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

/*
 * @func USART2_init
 * @brief To initialize USART2
 * @param void
 * @retval void
 */
void USART2_init(void)
{
    esp_log_level_set(TAG_USART2, ESP_LOG_INFO);

    // Select parameters to configure USART2
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver and configure USART2.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 40, &uart2_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    //Set USART2 pins 
    uart_set_pin(EX_UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Create a task to handler RX_USART2 events from ISR
    xTaskCreate(USART2_RX_Events_task, "uart_event_task", 2048, NULL, 12, NULL);
}

