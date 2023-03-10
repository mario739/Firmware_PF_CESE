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

xQueueHandle queue_data;
xQueueHandle queue_data_adquisition;
xQueueHandle queue_server_mqtt;
xQueueHandle queue_app;

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
                                        .mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL",},
										.status_mqtt_server=DOWN_CONECTION};

typedef enum
{
	CONFIG,
	DATA_ADQUISITION,
	UP_SERVER_MQTT,
	DOWN_SERVER_MQTT,
	SEND_DATA_SERVER_MQTT,
	LOW_POWER,
}en_state_machine_app;

typedef struct
{
	en_state_machine_app event_conetion;
}ts_event_app;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	ts_event_app event_app={.event_conetion=UP_SERVER_MQTT};
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;
	if (htim->Instance==htim6.Instance){
		xQueueSendFromISR(queue_app,&event_app,&xHigherPriorityTaskWoken);
		event_app.event_conetion=SEND_DATA_SERVER_MQTT;
		xQueueSendFromISR(queue_app,&event_app,&xHigherPriorityTaskWoken);
		event_app.event_conetion=DOWN_SERVER_MQTT;
		xQueueSendFromISR(queue_app,&event_app,&xHigherPriorityTaskWoken);
		HAL_TIM_Base_Stop_IT(htim);
	}
}

static void task_app_main(void *p_parameter)
{
	st_event_conection event_conection;
	st_event_data_adquisition event_data_adquisition;
	ts_event_app event_app;
	while(1)
	{
 		xQueueReceive(queue_app, &event_app, portMAX_DELAY);
		switch (event_app.event_conetion) {
			case CONFIG:
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
				HAL_TIM_Base_Start_IT(&htim6);
				break;
			case SEND_DATA_SERVER_MQTT:
				event_conection.event_conection=SEND_DATA_MQTT;
				xQueueSend(queue_server_mqtt,&event_conection,0);
				break;
			case LOW_POWER:
				//event_app2.event_conetion=LOW_POWER;
				//xQueueSend(queue_app,&event_app2,0);
				//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
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

	res=xTaskCreate(task_app_main, (const char*)"task_app_main", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	configASSERT(res == pdPASS);

	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	
	res = xTaskCreate(task_management_conection_server_mqtt, (const char*)"task_management_conection_server_mqtt", configMINIMAL_STACK_SIZE*2, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);


	queue_data=xQueueCreate(1, sizeof(struct st_data_sensors));
	queue_data_adquisition=xQueueCreate(1,sizeof(st_event_data_adquisition));
	queue_server_mqtt=xQueueCreate(3,sizeof(st_event_conection));
	queue_app=xQueueCreate(3,sizeof(ts_event_app));

	HAL_TIM_Base_Start_IT(&htim6);
	osKernelStart();

	while(1)
	{

	}
	return 1;
}


