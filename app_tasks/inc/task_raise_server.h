/*
 * task_raise_server.h
 *
 *  Created on: 7 feb. 2023
 *      Author: ferna
 */

#ifndef INC_TASK_RAISE_SERVER_H_
#define INC_TASK_RAISE_SERVER_H_

#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "adc.h"
#include "task_data_acquisition.h"


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern xQueueHandle queue_trama;
extern xQueueHandle queue_data;
extern st_bg96_config bg96_config;




struct st_config_mqtt_server
{
	char topic[30];
};
void task_raise_server(void *p_parameter);
em_bg96_error_handling write_data(char *command, char *request, char *buffer, uint32_t time);

#endif /* INC_TASK_RAISE_SERVER_H_ */
