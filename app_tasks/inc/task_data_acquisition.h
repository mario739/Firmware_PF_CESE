/*
 * task_data_acquisition.h
 *
 *  Created on: 7 feb. 2023
 *      Author: ferna
 */

#ifndef INC_TASK_DATA_ACQUISITION_H_
#define INC_TASK_DATA_ACQUISITION_H_

#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "aht10.h"
#include "callback_functions.h"
#include "adc.h"


typedef enum
{
	ADQUISITION
}en_states_data_adquisition;

struct st_data_sensors{
	uint8_t ambient_humidity;
	int8_t ambient_temperature;
	uint16_t batery;
	uint16_t radiacion;
	uint16_t soil_moisture_1;
	uint16_t soil_moisture_2;
};

typedef struct
{
	en_states_data_adquisition states_data_adquisition;
}st_event_data_adquisition;

void task_data_acquisition(void *p_parameter);
#endif /* INC_TASK_DATA_ACQUISITION_H_ */
