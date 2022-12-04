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

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

xQueueHandle queue_data;
xQueueHandle queue_trama;


aht10_config_t aht_config;

st_bg96_config bg96_config={.send_data_device=NULL,
                              .last_error=BG96_NO_ERROR,
                              .self_tcp={ .context_id=1,
                                          .context_type=1,
                                          .method_authentication=1,
                                          .tcp_apn="4g.entel",
                                          .tcp_password="",
                                          .tcp_username=""},
                              .self_mqtt={.identifier_socket_mqtt=0,
                                          .quality_service=0,
                                          .host_name="\"industrial.api.ubidots.com\"",//industrial.api.ubidots.com
                                          .port=1883,
                                          .mqtt_client_id="123456789",
                                          .mqtt_username="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL",
                                          .mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL"}};
uint8_t rx_tem[20];
uint8_t rx_buffer[300];
uint8_t rx_index=0;

char topic[]="/v1.6/devices/monitoreo_iot";
//{"temperatura_ambiente":10,"bateria":80,"radiacion":10,"humedad":80,"humedad":70,"humedad_ambiente":10}
char data[]="{\"bateria\":10,\"humedad\":60,\"humedad_ambiente\":60,\"radiacion\":10,\"temperatura_ambiente\":10}";

struct st_data_sensors{
	uint8_t ambient_humidity;
	int8_t ambient_temperature;
	uint8_t batery;
	uint8_t radiacion;
	uint8_t soil_moisture;
};


em_bg96_error_handling write_data(char *command, char *request, char *buffer, uint32_t time)
{
	HAL_UART_Transmit(&huart1, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Transmit(&huart2, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Receive_IT(&huart1,rx_tem,1);
	for (uint32_t i = 0; i < time; ++i)
	{
		vTaskDelay(1);

		if (strstr((char*)rx_buffer, request) != NULL)
		{
			HAL_UART_Transmit(&huart2, rx_buffer,strlen((const char *)rx_buffer),100);
			memset((char*)rx_buffer, 0, sizeof(rx_buffer));
			rx_index=0;
			return FT_BG96_OK;
		}
	}
	HAL_UART_AbortReceive_IT(&huart1);
	return FT_BG96_TIMEOUT;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart==&huart1)
	{
		rx_buffer[rx_index]=rx_tem[0];
		rx_index++;
		HAL_UART_Receive_IT(&huart1,rx_tem,1);
	}
}

static void task_data_acquisition(void *p_parameter)
{

	struct st_data_sensors data_sensors;
	while (1)
	{
		aht10_get_humedity(&aht_config,&data_sensors.ambient_humidity);
		aht10_get_temperature(&aht_config,&data_sensors.ambient_temperature);
		xQueueSend(queue_data,&data_sensors,portMAX_DELAY);
		vTaskDelay(20);
	}
}

static void task_data_concatenate(void *p_parameter)
{
	struct st_data_sensors data_sensors2;
	static char data2[150];
	while(1)
	{
		xQueueReceive(queue_data, &data_sensors2,portMAX_DELAY);
		sprintf(data2,"\"bateria\":%u,\"humedad\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperatura_ambiente\":%u}",data_sensors2.batery,
				data_sensors2.soil_moisture,data_sensors2.ambient_humidity,data_sensors2.radiacion,data_sensors2.ambient_temperature);
		//xQueueSend(queue_trama,data2,200);
		vTaskDelay(20);
	}
}

static void task_raise_server(void *p_parameter)
{
	static char* data2;

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
						//xQueueReceive(queue_trama,&data2,300);
						if (send_data_mqtt(&bg96_config,topic,data2)==FT_BG96_OK)
						{

						}
					}
					desactivate_context_pdp(&bg96_config);
				}
			}
		}

		vTaskDelay(200);
	}
}


int app(void)
{
	BaseType_t res;
    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);
    init_driver(&bg96_config,write_data);

	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE * 2, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	res=xTaskCreate(task_raise_server, (const char*)"task_raise_server", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL);
	configASSERT(res == pdPASS);
	res=xTaskCreate(task_data_concatenate, (const char*)"task_data_concatenate", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);

	queue_data=xQueueCreate(4, sizeof(struct st_data_sensors));
	queue_trama=xQueueCreate(4, sizeof(char *));

	osKernelStart();

	while(1)
	{

	}
	return 1;
}


