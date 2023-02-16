/**
 * @file app.c
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string.h>
#include <stdio.h>
#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "task_data_acquisition.h"
#include "task_raise_server.h"
#include "task_debug.h"
#include "task_events_fails.h"

xQueueHandle queue_data;
xQueueHandle queue_trama;
xQueueHandle queue_debug;

aht10_config_t aht_config;

uint8_t data_debug;

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

int app(void)
{
	BaseType_t res;
    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);
    init_driver(&bg96_config,write_data);
    HAL_TIM_Base_Start_IT(&htim6);

	res=xTaskCreate(task_raise_server, (const char*)"task_raise_server", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);
	configASSERT(res == pdPASS);

	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	
	res = xTaskCreate(task_debug, (const char*)"task_debug", (uint16_t)50, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);

	res=xTaskCreate(task_events_fails, (const char*)"task_events_fails", (uint16_t)50, NULL, tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);

	queue_data=xQueueCreate(3, sizeof(struct st_data_sensors));
	queue_debug=xQueueCreate(2,sizeof(void*));
	queue_trama=xQueueCreate(4, sizeof(char *));

	osKernelStart();

	while(1)
	{

	}
	return 1;
}


