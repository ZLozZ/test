#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_sleep.h"
#include <sys/param.h>

#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "esp_timer.h"
#include <inttypes.h>

#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "input_Loo.h"
#include "output.h"
#include "app_config.h"
#include "i2cdev.h"
#include "ds1307.h"
#include "cJSON.h"

#define CONFIG_SCL_GPIO		26
#define CONFIG_SDA_GPIO		25

#define DEN 13
#define QUAT 12
#define NUT1 14
#define NUT2 27
#define LED 2
#define BTN 18

static const char *TAG = "DS1307";
#define	CONFIG_TIMEZONE		7

static const char *TAG2 = "Sonoff";

RTC_DATA_ATTR static int boot_count = 0;
int year,mon,day, hour, min, sec;


unsigned int HenGioDen = 0;

unsigned int HenGioQuat = 0;

int GD_Den = 0;
int PD_Den = 0;
int GT_Den = 0;
int PT_Den = 0;

int GD_Quat = 0;
int PD_Quat = 0;
int GT_Quat = 0;
int PT_Quat = 0;

int GioThuc = 0;

int PhutThuc = 0;



int nhietdo = 0;
int doam = 0;
int TB1 = 0;
int TB2 = 0;
int C1 = 100;
int C2 = 200;
char JSON[100];
char Str_ND[100];
char Str_DA[100];
char Str_TB1[100];
char Str_TB2[100];
char Str_C1[100];
char Str_C2[100];
char Length[100];



long lastV0 = 0;
long lastV1 = 0;
long lastV2 = 0;
long lastV3 = 0;
long lastV4 = 0;
long last1 = 0;
//last1 = esp_timer_get_time()/1000;

int bienled = 0;
int start= 0;

static const char *TAG3 = "MQTT_EXAMPLE";
cJSON *str_json,*str_TB1 , *str_TB2 , *str_C1 , *str_C2 ,*usr;


void delay(uint32_t time);
void getClock(void *pvParameters);
void setClock(void *pvParameters);
void config_wifi(void);
void config_button(void);
void config_output(void);
void functionButton(void *arg);
void functionsensor(void *arg);
void functionLedIO2(void *arg);
void button(void);
void chuongtrinhcambien(void);
void functionConnectSendData(void *arg);
void DataJson(unsigned int ND , unsigned int DA,  unsigned int TB1,  unsigned int TB2, unsigned int C1, unsigned int C2, unsigned int H1, unsigned int H2);
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG3, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "tranconghoa/maylanh", "Hello Server!!!", 0, 1, 0);
            ESP_LOGI(TAG3, "sent publish successful, msg_id=%d", msg_id);
			start = msg_id;

            msg_id = esp_mqtt_client_subscribe(client, "tranconghoa/quat", 0);
            ESP_LOGI(TAG3, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG3, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG3, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG3, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            
			printf("Messager MQTT: %.*s\r\n",event->data_len,event->data);
			
			char *bufData = calloc(event->data_len + 1, sizeof(char));
			
			snprintf(bufData, event->data_len + 1,  "%s", event->data);
			
			printf("%s\n",bufData);
			
			// xử lý json
			
			str_json = cJSON_Parse(bufData);
			/*
			Nếu str_json trả về False lỗi dữ liệu JSON
			*/
			if(!str_json)
			{
				printf("Data JSON ERROR!!!\n");
				
			}
			else
			{
				printf("Data JSON OK!!!\n");
				
				// xử lý theo mục đích của mình
				
				// 	{"TB1":"1"}  {"TB1":"0"}
				
				//	{"TB2":"1"}  {"TB2":"0"}
				
				if(cJSON_GetObjectItem(str_json, "TB1"))
				{
					printf("Check TB1 OKE!!\n");
					
					// strstr
					
					if(strstr( cJSON_GetObjectItem(str_json, "TB1")->valuestring,"0") != NULL )
					{
						gpio_set_level(DEN, 0);
						TB1 = 0;
						printf("OFF 1!!!\n");
					}
					else if(strstr( cJSON_GetObjectItem(str_json, "TB1")->valuestring,"1") != NULL )
					{
						gpio_set_level(DEN, 1);
						TB1 = 1;
						printf("ON 1!!!\n");
					}					
				
				
				
				}
				
				
				
				if(cJSON_GetObjectItem(str_json, "TB2"))
				{
					printf("Check TB2 OKE!!\n");
					
				
					
					if(strstr( cJSON_GetObjectItem(str_json, "TB2")->valuestring,"0") != NULL )
					{
						gpio_set_level(QUAT, 0);
						TB2 = 0;
						printf("OFF 2!!!\n");
					}
					else if(strstr( cJSON_GetObjectItem(str_json, "TB2")->valuestring,"1") != NULL )
					{
						gpio_set_level(QUAT, 1);
						TB2 = 1;
						printf("ON 2!!!\n");
					}					
				}
				
				if(cJSON_GetObjectItem(str_json, "C1"))
				{
					printf("Check C1 OKE!!\n");
					
					C1 = atoi(cJSON_GetObjectItem(str_json, "C1")->valuestring);

					//nhietdonguong = atoi(cJSON_GetObjectItem(str_json, "C2")->valuestring);
					printf("OK C1: %d\r\n",C1);					
				}
				
				
				if(cJSON_GetObjectItem(str_json, "C2"))
				{
					printf("Check C2 OKE!!\n");
					
					C2 = atoi(cJSON_GetObjectItem(str_json, "C2")->valuestring);

					//nhietdonguong = atoi(cJSON_GetObjectItem(str_json, "C2")->valuestring);
					printf("OK C2: %d\r\n",C2);					
				}
				
			
				
				
				if(cJSON_GetObjectItem(str_json, "DEN"))
				{		
					HenGioDen = atoi(cJSON_GetObjectItem(str_json, "DEN")->valuestring);
					
					if(HenGioDen == 1)
					{
						if(cJSON_GetObjectItem(str_json, "GD"))
						{
							GD_Den = atoi(cJSON_GetObjectItem(str_json, "GD")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "PD"))
						{
							PD_Den = atoi(cJSON_GetObjectItem(str_json, "PD")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "GT"))
						{
							GT_Den = atoi(cJSON_GetObjectItem(str_json, "GT")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "PT"))
						{
							PT_Den = atoi(cJSON_GetObjectItem(str_json, "PT")->valuestring);
						}
						
						printf("Hẹn Giờ Đèn: %d:%d - %d:%d\n",GD_Den, PD_Den, GT_Den , PT_Den);
					}
					
				}
				
				if(cJSON_GetObjectItem(str_json, "QUAT"))
				{		
					HenGioQuat = atoi(cJSON_GetObjectItem(str_json, "QUAT")->valuestring);
					
					if(HenGioQuat == 1)
					{
						if(cJSON_GetObjectItem(str_json, "GD"))
						{
							GD_Quat = atoi(cJSON_GetObjectItem(str_json, "GD")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "PD"))
						{
							PD_Quat = atoi(cJSON_GetObjectItem(str_json, "PD")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "GT"))
						{
							GT_Quat = atoi(cJSON_GetObjectItem(str_json, "GT")->valuestring);
						}
						if(cJSON_GetObjectItem(str_json, "PT"))
						{
							PT_Quat = atoi(cJSON_GetObjectItem(str_json, "PT")->valuestring);
						}
						
						printf("Hẹn Giờ Quạt: %d:%d - %d:%d\n",GD_Den, PD_Den, GT_Den , PT_Den);
					}				
				}
							
				cJSON_Delete(str_json);		
					
			}
			
			
			free(bufData);
			
			last1 = esp_timer_get_time()/1000;
		
						
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG3, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void app_main(void)
{
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
  config_output();
  config_button();

  xTaskCreate(config_wifi, "task1", 2048*2, NULL, 3, NULL);
  xTaskCreate(getClock, "task1", 2048*2, NULL, 3, NULL);
  //tạo freertos 1 chạy hàm  kết nối + senddata
	xTaskCreate(functionConnectSendData, "task4", 2048*2, NULL, 3, NULL);
}

void functionsensor(void *arg)
{
	while(1)
	{
		chuongtrinhcambien();	
		delay(1000);
	}
	vTaskDelete(NULL);
}

void functionConnectSendData(void *arg)
{
	
	// kết nối MQTT
	// mqtt://user:pass@ địa chỉ server : port
	/*
	address: mqtt.ngoinhaiot.com
	port : 1111
	user: tranconghoa
	pass: E60A49FF2FBF4EEF
	*/
	
	esp_mqtt_client_config_t mqtt_cfg = {
		
        .uri = "mqtt://tranconghoa:E60A49FF2FBF4EEF@mqtt.ngoinhaiot.com:1111",
    };
	
	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
	
	
	last1 = esp_timer_get_time()/1000;
	
	
	while(1)
	{
		if(esp_timer_get_time()/1000 - last1 >= 1000)
		{
			if(MQTT_EVENT_CONNECTED)
			{
				DataJson(nhietdo , doam,  TB1,  TB2, C1, C2, HenGioDen , HenGioQuat);			
				esp_mqtt_client_publish(client, "tranconghoa/maylanh", JSON, 0, 1, 0);
			}
			
			last1 = esp_timer_get_time()/1000;
		}
		
		delay(50);
	}
	vTaskDelete(NULL);
}

void chuongtrinhcambien(void)
{

	nhietdo++;
	doam = doam + 2;
	printf("Nhiệt độ: %d\r\n",nhietdo);
	printf("Độ ẩm: %d\r\n",doam);
}

void DataJson(unsigned int ND , unsigned int DA,  unsigned int TB1,  unsigned int TB2, unsigned int C1, unsigned int C2 , unsigned int H1 ,unsigned int H2)
{
	for(int i = 0 ; i < 100; i++)
	{
		Str_ND[i] = 0;
		Str_DA[i] = 0;
		Str_TB1[i] = 0;
		Str_TB2[i] = 0;
		Str_C1[i] = 0;
		Str_C2[i] = 0;
		JSON[i] = 0;
	}
	
	sprintf(Str_ND, "%d", ND);
	sprintf(Str_DA, "%d", DA);
	sprintf(Str_TB1, "%d", TB1);
	sprintf(Str_TB2, "%d", TB2);
	sprintf(Str_C1, "%d", C1);
	sprintf(Str_C2, "%d", C2);
	// strcat(JSON,"{\"ND\":\""); => JSON = {{\"ND\":\"}
	//strcat ghep du lieu
	// {"ND":"","DA":""}
	strcat(JSON,"{\"ND\":\"");
	strcat(JSON,Str_ND);
	strcat(JSON,"\",");
	
	strcat(JSON,"\"DA\":\"");
	strcat(JSON,Str_DA);
	strcat(JSON,"\",");
	
	strcat(JSON,"\"TB1\":\"");
	strcat(JSON,Str_TB1);
	strcat(JSON,"\",");
	
	strcat(JSON,"\"TB2\":\"");
	strcat(JSON,Str_TB2);
	strcat(JSON,"\",");
	
	strcat(JSON,"\"C1\":\"");
	strcat(JSON,Str_C1);
	strcat(JSON,"\",");
	
	strcat(JSON,"\"C2\":\"");
	strcat(JSON,Str_C2);
	strcat(JSON,"\"}");
	strcat(JSON,"\r\n");
	
	printf("DataJson: %s\n", JSON);
}

void config_button(void)
{
  gpio_pad_select_gpio(BTN);
	gpio_set_direction(BTN, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BTN, GPIO_PULLUP_ONLY);
}

void config_output(void)
{
  output_io_create(LED);
}

void delay(uint32_t time)
{
	vTaskDelay(time / portTICK_PERIOD_MS);
}

void getClock(void *pvParameters)
{

	 i2c_dev_t dev;
    
    if (ds1307_init_desc(&dev, I2C_NUM_0, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO) != ESP_OK) 
	{
        ESP_LOGE(pcTaskGetTaskName(0), "Could not init device descriptor.");
        while (1) 
		{ 
			vTaskDelay(1); 
		}
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) 
	{
        struct tm time;

        if (ds1307_get_time(&dev, &time) != ESP_OK) 
		{
            ESP_LOGE(pcTaskGetTaskName(0), "Could not get time.");
            while (1) 
			{ 
				vTaskDelay(1); 
			}
        }
        year = time.tm_year;
        mon =  time.tm_mon;
        day = time.tm_mday;
        hour = time.tm_hour;
        min = time.tm_min;
        sec = time.tm_sec;
        ESP_LOGI(pcTaskGetTaskName(0), "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
		vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}

void setClock(void *pvParameters)
{


    i2c_dev_t dev;
    if (ds1307_init_desc(&dev, I2C_NUM_0, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO) != ESP_OK) 
	{
        ESP_LOGE(pcTaskGetTaskName(0), "Could not init device descriptor.");
        while (1) 
		{ 
			vTaskDelay(1); 
		}
    }



    struct tm time = {
        .tm_year = 2022,
        .tm_mon  = 12,  
        .tm_mday = 28,
        .tm_hour = 9,
        .tm_min  = 43,
        .tm_sec  = 50,
    };

    if (ds1307_set_time(&dev, &time) != ESP_OK) 
	{
        ESP_LOGE(pcTaskGetTaskName(0), "Could not set time.");
        while (1) 
		{ 
			vTaskDelay(1); 
		}
    }
    ESP_LOGI(pcTaskGetTaskName(0), "Set initial date time done");

    const int deep_sleep_sec = 1;
    ESP_LOGI(pcTaskGetTaskName(0), "Entering deep sleep for %d seconds", deep_sleep_sec);
    esp_deep_sleep(1000000LL * deep_sleep_sec);
    vTaskDelete(NULL);
}

void config_wifi(void)
{
	delay(100);
	int dem = 0;
    if(gpio_get_level(BTN) == 0)
    {
		delay(100);
		dem++;
    }
	if(dem >= 3)
	{
		output_io_set_level(LED, 1);
      	app_config();
	}
 	 output_io_set_level(LED, 0);
}
