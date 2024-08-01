/*********************************************************************
 * Description: Graduation Project
 * Project: To use NVS Flash to register and login WebServer
 * Author: Tran Doan Manh
 ********************************************************************/

    /* ------------------------- All libraries --------------------------*/
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

    /* ------------------------- Private macro --------------------------*/
    // To get registered information in NVS Flash
char register_data[20];                                 

/*
 * @func readString_NVSFlash
 * @brief To read registered information from NVS Flash
 * @param void
 * @retval void
 */
void readString_NVSFlash(void)
{
        // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

        // Open NVS_Flash
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);

        // Check error when opening NVS_Flash
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
        
            // Read string from NVS_Flash
        printf("Reading login from NVS ... ");
        size_t required_size = 0;
        nvs_get_str(my_handle, "login", NULL, &required_size);
        char *login_value = malloc(required_size);
        err = nvs_get_str(my_handle, "login", login_value, &required_size);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                strcpy(register_data, login_value);
                printf("Register_data: %s\n", register_data);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

            // Close
        nvs_close(my_handle);
    }
}

/*
 * @func readString_NVSFlash
 * @brief To write registration information to NVS Flash
 * @param logindata - registration information
 * @retval void
 */
void writeString_NVSFlash(char *login_data)
{
        // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
        // Open NVS_Flash
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);

        // Write string into NVS_Flash
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

            // Write data, key - "inf", value - "manh123"
        printf("Writing login into NVS ... ");
        err = nvs_set_str(my_handle, "login", login_data);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("Write data: %s\n", login_data);
                printf("Committing updates in NVS ... ");
                err = nvs_commit(my_handle);
                printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
                break;

            default :
                printf("Error (%s) writing!\n", esp_err_to_name(err));
        }
        
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        
            // Close
        nvs_close(my_handle);
    }
}

