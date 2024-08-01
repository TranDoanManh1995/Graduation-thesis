/*********************************************************************
 * Description: Graduation Project
 * Project: To use WebServerESP32 to communicate with Zigbee devices
 * Author: Tran Doan Manh
 ********************************************************************/

    /* ------------------------- All libraries --------------------------*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "Handle_ServerClient/Handle_ServerClient.h"

    /* ------------------------- Private macro --------------------------*/
    //@brief Relating to configure wifi
#define EXAMPLE_ESP_WIFI_SSID      "Phong Ke Hoach"
#define EXAMPLE_ESP_WIFI_PASS      "0942002030"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

    /* The event group allows multiple bits for each event, but we only care about two events:
    * - we are connected to the AP with an IP
    * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

    /* ------------------------- Global variable --------------------------*/
    //FreeRTOS event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;                         
static const char *TAG_MAIN = "WIFI STATION";
static int s_retry_num = 0;

/*
 * @func event_handler
 * @brief To process TCP/IP and Wifi events
 * @param arg - 
 *        event_base - The base ID of the event to register the handler for event_id
 *        event_id - The ID of the event to register the handler for event data
 *        event_data - To get ID address
 * @retval void
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
            // Wi-Fi driver will start the internal connection process
        esp_wifi_connect();                        
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
            // Retry to connect to the AP
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_MAIN, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_MAIN, "connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
            // Got IP when connecting successfully to AP
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_MAIN, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/*
 * @func wifi_init_sta
 * @brief To configure wifi at station mode
 * @param void
 * @retval void
 */
void wifi_init_sta(void)
{
        // FREETOS event group
    s_wifi_event_group = xEventGroupCreate();

        /*-------------------- Init Phase -----------------------*/
        // To initialize LightWeight TCP/IP and create LwIP task. It sends TCP/IP events to Event task
    ESP_ERROR_CHECK(esp_netif_init());

        // To initialize System Event task. It receives WIFI, TCP/IP events and send that events to Application task
    ESP_ERROR_CHECK(esp_event_loop_create_default());

        // Application task can register for a callback that listen for WiFi and TCP/IP events
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

        // Binding WiFi driver and TCP/IP stack (esp-netif or lwIP)
    esp_netif_create_default_wifi_sta();

        // Create WiFi driver task and initialise WiFi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        /*-------------------- Configure Phase -----------------------*/
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

        /*-------------------- Start Phase -----------------------*/
        /* To start WiFi and connect to an AP
        *  Wi-Fi driver posts WIFI_EVENT_STA_START to the event task
        * relays the WIFI_EVENT_STA_START to the application task */
    ESP_ERROR_CHECK(esp_wifi_start() );                        

    ESP_LOGI(TAG_MAIN, "wifi_init_sta finished.");

        /* After initialising and starting WiFi, the application task 
        goes to Blocked state waiting for bits in event group to be set*/
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_MAIN, "connected to ap SSID: %s password: %s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_MAIN, "Failed to connect to SSID: %s, password: %s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG_MAIN, "UNEXPECTED EVENT");
    }
}

/*
 * @func app_main
 * @brief main
 * @param void
 * @retval void
 */
void app_main(void)
{
        // Initialize NVS to store Wifi Configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

        // To configure wifi at station mode
    ESP_LOGI(TAG_MAIN, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

        // Initialize USART2
    USART2_init();

        // Run WebServer
    start_webserver();
}