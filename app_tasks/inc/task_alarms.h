/*
 * task_alarms.h
 *
 *  Created on: Aug 22, 2023
 *      Author: mario
 */

#ifndef INC_TASK_ALARMS_H_
#define INC_TASK_ALARMS_H_

#include "app.h"
#include "cmsis_os.h"
#include "task_data_acquisition.h"


typedef enum
{
	MONITOR,
	SEND
}en_event_alarms;

typedef struct
{
	char *type_alarms;
	st_data_sensors sensors_data;
	en_event_alarms event_alarms;

}st_config_alarms;


void task_alarms(void *p_parameter);

#endif /* INC_TASK_ALARMS_H_ */
