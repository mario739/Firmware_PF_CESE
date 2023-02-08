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
#include "driver_bg96.h"
#include "callback_functions.h"
#include "adc.h"

struct st_data_sensors{
	uint8_t ambient_humidity;
	int8_t ambient_temperature;
	uint8_t batery;
	uint8_t radiacion;
	uint8_t soil_moisture_1;
	uint8_t soil_moisture_2;
};

void task_data_acquisition(void *p_parameter);
#endif /* INC_TASK_DATA_ACQUISITION_H_ */
