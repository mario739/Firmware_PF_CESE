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
	static st_config_alarms config_alarms;
	char numero[9]="72950576";
	char message[25]="Humedad de suelo muy baja";
	static uint8_t active_alarms=0;
	while(1)
	{
		xQueueReceive(queue_alarms,&config_alarms,portMAX_DELAY);
		switch (config_alarms.event_alarms)
		{
			case MONITOR:
				if (config_alarms.sensors_data.soil_moisture_1 <= 10)
				{
					active_alarms=1;
				}
				break;
			case SEND:
				if (active_alarms==1)
				{
					set_sms_format(&bg96_config,1);
					send_sms_bg96(&bg96_config,numero,message);
					active_alarms=0;
				}
				xSemaphoreGive(semaphore_loop);
				break;
			default:
				break;
		}
	}
}
