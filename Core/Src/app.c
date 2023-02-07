/*
 * app.c
 *
 *  Created on: 1 sep. 2022
 *      Author: Mario Aguilar
 */


#include <string.h>
#include <stdio.h>
#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "adc.h"


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

xQueueHandle queue_data;
xQueueHandle queue_trama;

uint32_t map(uint32_t value, uint32_t inputMin, uint32_t inputMax, uint32_t outputMin, uint32_t outputMax);

aht10_config_t aht_config;

st_bg96_config bg96_config={.send_data_device=NULL,
                            .last_error=BG96_NO_ERROR,
                            .self_tcp={.context_id=1,
                                       .context_type=1,
                                       .method_authentication=1,
                                       .tcp_apn="4g.entel",
                                       .tcp_password="",
                                       .tcp_username=""},
                            .self_mqtt={.identifier_socket_mqtt=0,
                                        .quality_service=0,
                                        .host_name="\"industrial.api.ubidots.com\"",
                                        .port=1883,
                                        .mqtt_client_id="123a56789",
                                        .mqtt_username="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL",
                                        .mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL"}};
uint8_t  rx_tem;
uint8_t rx_buffer[300];
uint16_t rx_index=0;

struct st_data_sensors{
	uint8_t ambient_humidity;
	int8_t ambient_temperature;
	uint8_t batery;
	uint8_t radiacion;
	uint8_t soil_moisture_1;
	uint8_t soil_moisture_2;
};

struct st_config_mqtt_server
{
	char topic[30];
};

static em_bg96_error_handling write_data(char *command, char *request, char *buffer, uint32_t time)
{
	HAL_UART_Transmit(&huart1, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Transmit(&huart2, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	char* res;
	for (uint32_t i = 0; i < time; ++i)
	{
		vTaskDelay(1);
		res=strstr((char*)rx_buffer,request);
		if (res!= NULL)
		{
			HAL_UART_Transmit(&huart2, rx_buffer,strlen((const char *)rx_buffer),200);
			memset((char*)rx_buffer, 0, sizeof(rx_buffer));
			rx_index=0;
			return FT_BG96_OK;
		}
	}
	rx_index=0;
	HAL_UART_Transmit(&huart2, rx_buffer,strlen((const char *)rx_buffer),200);
	memset((char*)rx_buffer, 0, sizeof(rx_buffer));
	return FT_BG96_TIMEOUT;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart==&huart1)
	{
		rx_buffer[rx_index]=(char)rx_tem;
		rx_index++;
		rx_tem=0;
		HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	}
}

static void task_data_acquisition(void *p_parameter)
{
	struct st_data_sensors data_sensors={.ambient_humidity=20,.ambient_temperature=20,.batery=30,.radiacion=10,.soil_moisture_1=20};
	while (1)
	{
		aht10_get_humedity(&aht_config,&data_sensors.ambient_humidity);
		aht10_get_temperature(&aht_config,&data_sensors.ambient_temperature);
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.soil_moisture_1= map(HAL_ADC_GetValue(&hadc1), 2700, 4001, 0, 100);
		data_sensors.soil_moisture_1= 100 - data_sensors.soil_moisture_1;
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.soil_moisture_2= map(HAL_ADC_GetValue(&hadc1), 2700, 4001, 0, 100);
		data_sensors.soil_moisture_2= 100 - data_sensors.soil_moisture_2;
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.radiacion= map(HAL_ADC_GetValue(&hadc1), 1000, 2500, 0, 15);
		HAL_ADC_Stop(&hadc1);
		xQueueSend(queue_data,&data_sensors,portMAX_DELAY);
	}
}
uint32_t map(uint32_t value, uint32_t inputMin, uint32_t inputMax, uint32_t outputMin, uint32_t outputMax)
{
  return ((((value - inputMin) * (outputMax - outputMin)) / (inputMax - inputMin)) + outputMin);
}

/*
static void task_data_concatenate(void *p_parameter)
{
	struct st_data_sensors data_sensors2;
	static char data2[150];
	while(1)
	{
		xQueueReceive(queue_data, &data_sensors2,portMAX_DELAY);
		sprintf(data2,"\"bateria\":%u,\"humedad\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperatura_ambiente\":%u}",data_sensors2.batery,
		data_sensors2.soil_moisture,data_sensors2.ambient_humidity,data_sensors2.radiacion,data_sensors2.ambient_temperature);
		xQueueSend(queue_trama,data2,200);
		vTaskDelay(20);
	}
}*/

static void task_raise_server(void *p_parameter)
{
	struct st_data_sensors data_sensors2;
	struct st_config_mqtt_server config_mqtt_server={.topic="/v1.6/devices/monitoreo_iot"};
	char data[200];
	while(1)
	{
		if (get_status_modem(&bg96_config)==FT_BG96_OK)
		{
			if (get_status_sim(&bg96_config)==FT_BG96_OK)
			{
				if (set_parameter_context_tcp(&bg96_config)==FT_BG96_OK)
				{
					if (activate_context_pdp(&bg96_config)==FT_BG96_OK)
					{
						xQueueReceive(queue_data,&data_sensors2,300);
						sprintf(data,"\"bateria\":%u,\"humedad\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperatura_ambiente\":%u}",data_sensors2.batery,
						data_sensors2.soil_moisture_1,data_sensors2.ambient_humidity,data_sensors2.radiacion,data_sensors2.ambient_temperature);
						if (send_data_mqtt(&bg96_config,config_mqtt_server.topic,data)==FT_BG96_OK)
						{

						}
					}
					desactivate_context_pdp(&bg96_config);
				}
			}
		}

		vTaskDelay(100);
	}
}


int app(void)
{
	BaseType_t res;
    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);
    init_driver(&bg96_config,write_data);
	res=xTaskCreate(task_raise_server, (const char*)"task_raise_server", configMINIMAL_STACK_SIZE*3 , NULL, tskIDLE_PRIORITY + 2, NULL);
	configASSERT(res == pdPASS);

	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	/*
	res=xTaskCreate(task_data_concatenate, (const char*)"task_data_concatenate", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);*/

	queue_data=xQueueCreate(1, sizeof(struct st_data_sensors));
	queue_trama=xQueueCreate(4, sizeof(char *));


	osKernelStart();

	while(1)
	{

	}
	return 1;
}


