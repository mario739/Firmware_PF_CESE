/**
 * @file task_debug.c
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "task_debug.h"

extern UART_HandleTypeDef huart2;
extern xQueueHandle queue_debug;
extern uint8_t data_debug;


void task_debug(void *p_parameter)
{
	while(1)
	{
//		xQueueReceive(queue_debug,(void*)&data_debug_2,portMAX_DELAY);
		//HAL_UART_Transmit(&huart2,&data_debug_2, strlen(&data_debug_2),50);
		//memset(data_debug_2,0,200);
	}


}
