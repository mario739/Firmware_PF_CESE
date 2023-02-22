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
xQueueHandle queue_data_adquisition;
xQueueHandle queue_server_mqtt;
xQueueHandle queue_app;


aht10_config_t aht_config;

uint8_t data_debug;

TaskHandle_t xHandle_raise_server;
TaskHandle_t xHandle_data_adquisition;

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
                                        .mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL",},
										.status_mqtt_server=DOWN_CONECTION};

typedef enum
{
	CONFIG,
	PROCES,
	LOW_POWER,
}en_state_machine_app;

typedef struct
{
	en_state_machine_app event_conetion;
}ts_event_app;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	ts_event_app event_app={.event_conetion=PROCES};
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;
	if (htim->Instance==htim6.Instance){
		xQueueSendFromISR(queue_app,&event_app,&xHigherPriorityTaskWoken);
		HAL_TIM_Base_Stop(htim);
	}
}

static void task_app_main()
{
	st_event_conection event_conection;
	st_event_data_adquisition event_data_adquisition;
	ts_event_app event_app;
	ts_event_app event_app2;
	while(1)
	{
 		xQueueReceive(queue_app, &event_app, portMAX_DELAY);
		switch (event_app.event_conetion) {
			case CONFIG:
				break;
			case PROCES:
				event_data_adquisition.states_data_adquisition=ADQUISITION;
				xQueueSend(queue_data_adquisition,&event_data_adquisition,0);
				event_conection.event_conection=UP_CONECTION;
				xQueueSend(queue_server_mqtt,&event_conection,0);
				if (bg96_config.status_mqtt_server==SERVER_MQTT_UP)
				{
					event_conection.event_conection=SEND_DATA_MQTT;
					xQueueSend(queue_server_mqtt,&event_conection,0);
				}
				event_conection.event_conection=DOWN_CONECTION;
				xQueueSend(queue_server_mqtt,&event_conection,portMAX_DELAY);
				event_app2.event_conetion=LOW_POWER;
				xQueueSend(queue_app,&event_app2,0);
				break;
			case LOW_POWER:
				HAL_TIM_Base_Start_IT(&htim6);
				HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
				break;
			default:
				break;
		}
	}
}
int app(void)
{
	BaseType_t res;

    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);
    init_driver(&bg96_config,write_data,reset_modem);

	res=xTaskCreate(task_app_main, (const char*)"task_app_main", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	configASSERT(res == pdPASS);

	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, &xHandle_data_adquisition);
	configASSERT(res == pdPASS);
	
	res = xTaskCreate(task_management_conection_server_mqtt, (const char*)"task_debug", configMINIMAL_STACK_SIZE*2, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);


	queue_data=xQueueCreate(3, sizeof(struct st_data_sensors));
	queue_data_adquisition=xQueueCreate(2,sizeof(st_event_data_adquisition));
	queue_trama=xQueueCreate(4, sizeof(char *));
	queue_server_mqtt=xQueueCreate(1,sizeof(st_event_conection));
	queue_app=xQueueCreate(2,sizeof(ts_event_app));

	HAL_TIM_Base_Start_IT(&htim6);
	osKernelStart();

	while(1)
	{

	}
	return 1;
}


