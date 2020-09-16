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
#include "driver/gpio.h"
#include "sdkconfig.h"

//#include "waiter.h"
#include "dht11.h"
/**
 * DefiniÃ§Ãµes Gerais
 */
#define DEBUG 1
#define CONFIG_WIFI_SSID        "9 de julho"
#define CONFIG_WIFI_PASSWORD    "support0"  

#define BUTTON	GPIO_NUM_17 
#define TOPICO_TEMPERATURA      "temperature"
#define TOPICO_UMIDADE          "humidity"



static const char *TAG = "main: ";
static EventGroupHandle_t wifi_event_group;
static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;
esp_mqtt_client_handle_t client;

static esp_err_t mqtt_event_handler( esp_mqtt_event_handle_t event );
static esp_err_t wifi_event_handler( void *ctx, system_event_t *event );
static void wifi_init_sta( void );
void task_sensor( void *pvParameter );
void app_main( void );
static void mqtt_app_start( void );


static esp_err_t mqtt_event_handler( esp_mqtt_event_handle_t event )
{
   
    client = event->client;
    
    switch (event->event_id) 
    {

          case MQTT_EVENT_BEFORE_CONNECT: 

            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            
          break;

        case MQTT_EVENT_CONNECTED:

        	if( DEBUG )
            	ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        
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

 	        
	            ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
	            ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);       		
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

    ESP_ERROR_CHECK( esp_event_loop_init( wifi_event_handler, NULL ) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    if( DEBUG )
    {
     	ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    	ESP_LOGI(TAG, "Waiting for wifi");   	
    }

}

void task_sensor( void *pvParameter )
{
    char buff_temp[20]; 
    char buff_umid[20];
  
    for(;;) 
	  {
        		
		xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);//aguarda para sempre a conexão 
		
		DHT11_init(GPIO_NUM_4);
        sprintf( buff_temp, "%d Celsius", DHT11_read().temperature);
        sprintf( buff_umid, "%d%%", DHT11_read().humidity);
        
		esp_mqtt_client_publish( client, TOPICO_TEMPERATURA, buff_temp, strlen(buff_temp ), 0, 0 );
        vTaskDelay( 2000/portTICK_PERIOD_MS );
		esp_mqtt_client_publish( client, TOPICO_UMIDADE, buff_umid, strlen(buff_umid ), 0, 0 );
		ESP_LOGI(TAG, "Enviado temperatura e umidade com sucesso\n\r");				
  		/**
  		 * Contribui para as demais tasks de menor prioridade sejam escalonadas
  		 * pelo scheduler;
  		 */
  		vTaskDelay( 100/portTICK_PERIOD_MS );	
    }
}

void app_main( void )
{
     
    esp_err_t ret = nvs_flash_init();
    //nvs_flash_erase();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    
    wifi_event_group = xEventGroupCreate(); //variavel global
 
    mqtt_event_group = xEventGroupCreate();//variável global

    wifi_init_sta(); //configura o ESP-32 como modo cliente
   
    xEventGroupWaitBits( wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY );
 
    mqtt_app_start();

    if( xTaskCreate( task_sensor, "task_sensor", 10000, NULL, 1, NULL ) != pdTRUE );
    {
      if( DEBUG )
         ESP_LOGI( TAG, "error - Nao foi possivel alocar task_sensor.\r\n" );  
      return; 
    }
}
    	

