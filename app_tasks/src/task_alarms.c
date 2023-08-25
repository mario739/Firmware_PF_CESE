/*
 * task_alarms.c
 *
 *  Created on: Aug 22, 2023
 *      Author: mario
 */

#include "task_alarms.h"
#include "driver_bg96.h"

extern xQueueHandle queue_alarms;
extern SemaphoreHandle_t semaphore_loop;
extern st_bg96_config bg96_config;


void task_alarms(void *p_parameter)
{
	st_event_alarms event_alarms;
	uint8_t alarms=0;
	while(1)
	{
		xQueueReceive(queue_alarms,&event_alarms,portMAX_DELAY);
		switch (event_alarms.event_alarms)
		{
			case MONITOR:
				if (event_alarms.sensors_data.soil_moisture_1 < 5)
				{
					alarms=1;
				}
				break;
			case SEND:
				if (alarms==1) {
					set_sms_format(&bg96_config,1);
					send_sms_bg96(&bg96_config,"72950576","Humedad de suelo muy baja");
					alarms=0;
				}
					xSemaphoreGive(semaphore_loop);
				break;
			default:
				break;
		}
	}
}
