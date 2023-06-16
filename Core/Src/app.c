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
#include <task_management_conection_server_mqtt.h>
#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "task_data_acquisition.h"

xQueueHandle queue_dispacher;
xQueueHandle queue_loop;

xQueueHandle queue_data;
xQueueHandle queue_data_adquisition;
xQueueHandle queue_server_mqtt;
SemaphoreHandle_t semaphore_loop;

aht10_config_t aht_config;
st_bg96_config bg96_config;
typedef enum
{
	LOOP,
	DATA_ADQUISITION,
	UP_SERVER_MQTT,
	DOWN_SERVER_MQTT,
	SEND_DATA_SERVER_MQTT,
	LOW_POWER,
}en_state_machine_app;

typedef struct
{
	en_state_machine_app events_system;
}ts_event_system;

typedef struct
{
	int data;
}ts_event_loop;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;
	ts_event_system event_system;
	event_system.events_system=LOOP;
	xQueueSendFromISR(queue_dispacher,&event_system,0);
    HAL_TIM_Base_Stop_IT(htim);

}

static void task_event_dispacher(void *p_parameter)
{
	st_event_conection event_conection;
	st_event_data_adquisition event_data_adquisition;
	ts_event_system event_system;
	ts_event_loop event_loop;
	HAL_TIM_Base_Start_IT(&htim6);
	while(1)
	{
 		xQueueReceive(queue_dispacher, &event_system, portMAX_DELAY);
		switch (event_system.events_system) {
			case LOOP:
				xQueueSend(queue_loop,&event_loop,0);
				break;
			case DATA_ADQUISITION:
				event_data_adquisition.states_data_adquisition=ADQUISITION;
				xQueueSend(queue_data_adquisition,&event_data_adquisition,0);
				break;
			case UP_SERVER_MQTT:
				event_conection.event_conection=UP_CONECTION;
				xQueueSend(queue_server_mqtt,&event_conection,0);
				break;
			case DOWN_SERVER_MQTT:
				event_conection.event_conection=DOWN_CONECTION;
				xQueueSend(queue_server_mqtt,&event_conection,0);
				break;
			case SEND_DATA_SERVER_MQTT:
				event_conection.event_conection=SEND_DATA_MQTT;
				xQueueSend(queue_server_mqtt,&event_conection,0);
				break;
			case LOW_POWER:
				break;
			default:
				break;
		}
	}
}

static void task_loop(void *p_parameter)
{
	ts_event_system event_dispacher;
	ts_event_loop event_loop;

	while(1)
	{
		xQueueReceive(queue_loop, &event_loop,portMAX_DELAY);
		event_dispacher.events_system=DATA_ADQUISITION;
		xQueueSend(queue_dispacher,&event_dispacher,portMAX_DELAY);
		event_dispacher.events_system=UP_SERVER_MQTT;
		xQueueSend(queue_dispacher,&event_dispacher,portMAX_DELAY);
		xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		if (bg96_config.status_mqtt_server==SERVER_MQTT_UP)
		{
			event_dispacher.events_system=SEND_DATA_SERVER_MQTT;
			xQueueSend(queue_dispacher,&event_dispacher,portMAX_DELAY);
			xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		}
		event_dispacher.events_system=DOWN_SERVER_MQTT,
		xQueueSend(queue_dispacher,&event_dispacher,portMAX_DELAY);
		xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		HAL_TIM_Base_Start_IT(&htim6);
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

int app(void)
{
	BaseType_t res;

    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);
    init_driver(&bg96_config,write_data,reset_modem);

	res=xTaskCreate(task_event_dispacher, (const char*)"task_event_dispache", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	configASSERT(res == pdPASS);

	res=xTaskCreate(task_loop, (const char*)"task_loop", 50, NULL, tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);

	res = xTaskCreate(task_management_conection_server_mqtt, (const char*)"task_management_conection_server_mqtt", configMINIMAL_STACK_SIZE*2, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	
	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", 50 , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);


	queue_dispacher=xQueueCreate(3,sizeof(ts_event_system));
	queue_loop=xQueueCreate(3,sizeof(st_event_conection));
	queue_data_adquisition=xQueueCreate(1,sizeof(st_event_data_adquisition));
	queue_server_mqtt=xQueueCreate(3,sizeof(st_event_conection));

	queue_data=xQueueCreate(1, sizeof(struct st_data_sensors));
	semaphore_loop=xSemaphoreCreateBinary();

	osKernelStart();

	while(1)
	{

	}
	return 1;
}


