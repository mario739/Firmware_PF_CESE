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

#include <task_management_conection_server_mqtt.h>
#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "task_data_acquisition.h"
#include "task_alarms.h"

xQueueHandle queue_data;
xQueueHandle queue_data_adquisition;
xQueueHandle queue_server_mqtt;
xQueueHandle queue_alarms;
extern UART_HandleTypeDef huart2;
SemaphoreHandle_t semaphore_loop;
SemaphoreHandle_t semaphore_timer;
aht10_config_t aht_config;
st_bg96_config bg96_config;
char data_error[50];

uint32_t data;
uint32_t data2;
uint32_t data3;
uint8_t port;
uint8_t port2;
static uint32_t map(uint32_t value, uint32_t inputMin, uint32_t inputMax, uint32_t outputMin, uint32_t outputMax)
{
	if(value<=inputMin){
		value=inputMin;
	}
	else if (value>=inputMax) {
		value=inputMax;
	}
  return ((((value - inputMin) * (outputMax - outputMin)) / (inputMax - inputMin)) + outputMin);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdTRUE;
    HAL_TIM_Base_Stop_IT(&htim6);
	xSemaphoreGiveFromISR(semaphore_timer,&xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void task_loop(void *p_parameter)
{
	st_event_conection event_conection;
	st_event_data_adquisition event_data_adquisition;
	st_config_alarms config_alarms;
	HAL_TIM_Base_Start_IT(&htim6);

	while(1)
	{
		xSemaphoreTake(semaphore_timer,portMAX_DELAY);
		event_data_adquisition.states_data_adquisition=ADQUISITION;
		xQueueSend(queue_data_adquisition,&event_data_adquisition,0);
		event_conection.event_conection=UP_CONECTION;
		xQueueSend(queue_server_mqtt,&event_conection,portMAX_DELAY);
		xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		if (bg96_config.status_mqtt_server==SERVER_MQTT_UP)
		{
			event_conection.event_conection=SEND_DATA_MQTT;
			xQueueSend(queue_server_mqtt,&event_conection,portMAX_DELAY);
			xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		}
		config_alarms.event_alarms=SEND;
		xQueueSend(queue_alarms,&config_alarms,portMAX_DELAY);
		xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		event_conection.event_conection=DOWN_CONECTION,
		xQueueSend(queue_server_mqtt,&event_conection,portMAX_DELAY);
		xSemaphoreTake(semaphore_loop,portMAX_DELAY);
		sprintf(data_error,"error en el sistema %u\r",bg96_config.last_error);
		HAL_UART_Transmit(&huart2, (const uint8_t*)data_error, strlen(data_error),50);
		HAL_TIM_Base_Start_IT(&htim6);
		/*HAL_SuspendTick();
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON , PWR_SLEEPENTRY_WFI);*/
	}
}


int app(void)
{
	BaseType_t res;

    init_driver(&bg96_config,write_data);
    aht10Init(&aht_config, write_I2C_STM32L432_port, read_I2C_STM32L432_port, delay_STM32L432_port);

	res=xTaskCreate(task_loop, (const char*)"task_loop", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	configASSERT(res == pdPASS);
	res = xTaskCreate(task_management_conection_server_mqtt, (const char*)"task_management_conection_server_mqtt", configMINIMAL_STACK_SIZE*2, NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	res = xTaskCreate(task_data_acquisition, (const char*)"task_data_acquisition", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);
	res = xTaskCreate(task_alarms, (const char*)"task_alarms", configMINIMAL_STACK_SIZE , NULL,tskIDLE_PRIORITY + 1, NULL);
	configASSERT(res == pdPASS);

	queue_data_adquisition=xQueueCreate(2,sizeof(st_event_data_adquisition));
	queue_server_mqtt=xQueueCreate(3,sizeof(st_event_conection));
	queue_data=xQueueCreate(2,sizeof(st_data_sensors));
	queue_alarms=xQueueCreate(2,sizeof(st_config_alarms));

	semaphore_loop=xSemaphoreCreateBinary();
	semaphore_timer=xSemaphoreCreateBinary();

	/*HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	data=HAL_ADC_GetValue(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	data2=HAL_ADC_GetValue(&hadc1);
	port=100-(map(data2, 1000,2100, 0,100));
	HAL_ADC_PollForConversion(&hadc1, 100);
	data3=HAL_ADC_GetValue(&hadc1);
	port2=11-(map(data3, 1000,1800, 0,11));
	HAL_ADC_Stop(&hadc1);*/

	osKernelStart();

	while(1)
	{

	}
	return 1;
}


