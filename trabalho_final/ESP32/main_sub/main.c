
/**
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
/**
 * WiFi
 */
#include "esp_wifi.h"
/**
 * WiFi Callback
 */
#include "esp_event_loop.h"
/**
 * Log
 */
#include "esp_system.h"
#include "esp_log.h"
/**
 * NVS
 */
#include "nvs.h"
#include "nvs_flash.h"
/**
 * LWIP
 */
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

/**
 * Lib MQTT
 */
#include "mqtt_client.h"

/**
 * Definições Gerais
 */
#define DEBUG 1
#define CONFIG_WIFI_SSID        "9 de julho"
#define CONFIG_WIFI_PASSWORD    "support0"

#define LED1  GPIO_NUM_33 
#define LED2  GPIO_NUM_32
#define TOPICO_LED1              "light"

/**
 * Variáveis
 */
esp_mqtt_client_handle_t client;
static const char *TAG = "main: ";
QueueHandle_t Queue_Button;
static EventGroupHandle_t wifi_event_group;
static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;


static esp_err_t wifi_event_handler( void *ctx, system_event_t *event );
static void wifi_init_sta( void );
static esp_err_t mqtt_event_handler( esp_mqtt_event_handle_t event );
static void mqtt_app_start( void );
void task_button( void *pvParameter );
void app_main( void );


static esp_err_t wifi_event_handler( void *ctx, system_event_t *event )
{
    switch( event->event_id ) 
    {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( wifi_event_group, CONNECTED_BIT );
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits( wifi_event_group, CONNECTED_BIT );
            break;

        default:
            break;
    }
    return ESP_OK;
}


static void wifi_init_sta( void )
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    if( DEBUG )
    {
        ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
        ESP_LOGI(TAG, "Waiting for wifi");      
    }

}


static esp_err_t mqtt_event_handler( esp_mqtt_event_handle_t event )
{
    int msg_id = 0;
	gpio_pad_select_gpio(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    

    client = event->client;
    
    switch (event->event_id) 
    {

        case MQTT_EVENT_BEFORE_CONNECT: 
            ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;

        
        case MQTT_EVENT_CONNECTED:

            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            
            msg_id = esp_mqtt_client_subscribe( client, TOPICO_LED1, 0 );
            
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            
            xEventGroupSetBits( mqtt_event_group, CONNECTED_BIT );
            break;
        
        case MQTT_EVENT_DISCONNECTED:

            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");   
            
            xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
            break;

        
        case MQTT_EVENT_SUBSCRIBED:
		   
		    break;
        
        
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        
    
        case MQTT_EVENT_PUBLISHED:
            
            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        
        
        case MQTT_EVENT_DATA:

            if( DEBUG )
            {
                ESP_LOGI(TAG, "MQTT_EVENT_DATA"); 

                ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);//event � um ponteiro
                ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data); 
					  
				
				if((event->data[0] == 'O')&&(event->data[1] == 'N'))
				{
					
					gpio_set_level( LED1, 1);
					ESP_LOGI(TAG, "led1 aceso");  
				
				}else if ((event->data[0] == 'O')&&(event->data[1] == 'F')&(event->data[2] == 'F'))
				
	 			{ 
	 				gpio_set_level( LED1, 0);
					ESP_LOGI(TAG, "led1 apagado");  	
	 			
				}
			 }
            break;
        
        case MQTT_EVENT_ERROR:
            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}


static void mqtt_app_start( void )
{

    const esp_mqtt_client_config_t mqtt_cfg = {
  		.uri =  "mqtt://192.168.15.39:1883",                                  
        .event_handle = mqtt_event_handler,
  		.username = "admin",
  		.password = "54321",
    };

    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init( &mqtt_cfg );
    esp_mqtt_client_start(client);
}


void app_main( void )
{
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	

	wifi_event_group = xEventGroupCreate();
	
	mqtt_event_group = xEventGroupCreate();
	
	wifi_init_sta();
	
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	
    mqtt_app_start();
	
}
