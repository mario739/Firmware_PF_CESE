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
#include "task_data_acquisition.h"
#include "task_raise_server.h"

xQueueHandle queue_data;
xQueueHandle queue_trama;

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
                                        .mqtt_client_id="123a56cb9",
                                        .mqtt_username="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL",
                                        .mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL"}};

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


