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
	while(1)
	{
		xQueueReceive(queue_alarms,&event_alarms,portMAX_DELAY);
		switch (event_alarms.event_alarms)
		{
			case MONITOR:
				if (event_alarms.sensors_data.ambient_humidity < 10 || event_alarms.sensors_data.ambient_temperature >60 ||
						event_alarms.sensors_data.radiacion>14 || event_alarms.sensors_data.soil_moisture_1 < 5)
				{
					st_event_alarms event_alarms2={.event_alarms=SEND,.sensors_data=event_alarms.sensors_data};
					xQueueSend(queue_alarms,&event_alarms2,0);
				}
				else {
					xSemaphoreGive(semaphore_loop);
				}
				break;
			case SEND:
				//send_sms_bg96(&bg96_config,"72950576","hola");
				xSemaphoreGive(semaphore_loop);
				break;
			default:
				break;
		}
	}
}
