

#include "task_data_acquisition.h"
#include "task_alarms.h"
#include "adc.h"

extern ADC_HandleTypeDef hadc1;
extern aht10_config_t aht_config;
extern xQueueHandle queue_data;
extern xQueueHandle queue_data_adquisition;
extern xQueueHandle queue_alarms;

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

void task_data_acquisition(void *p_parameter)
{
	st_event_data_adquisition event_data_adquisition;
	st_event_alarms event_alarms={.event_alarms=MONITOR};
	struct st_data_sensors data_sensors={.ambient_humidity=20,.ambient_temperature=20,.batery=30,.radiacion=10,.soil_moisture_1=20,.soil_moisture_2=0,.latitude=-21.5537892,.longitude=-64.747228};
	while (1)
	{
		xQueueReceive(queue_data_adquisition,&event_data_adquisition, portMAX_DELAY);
		switch (event_data_adquisition.states_data_adquisition)
		{
			case ADQUISITION:
 				aht10_get_humedity(&aht_config,&data_sensors.ambient_humidity);
				aht10_get_temperature(&aht_config,&data_sensors.ambient_temperature);
				HAL_ADC_Start(&hadc1);
				HAL_ADC_PollForConversion(&hadc1, 100);
				data_sensors.soil_moisture_1=HAL_ADC_GetValue(&hadc1);
				data_sensors.soil_moisture_1=map(data_sensors.soil_moisture_1, 1300,2700, 0,100);
				HAL_ADC_PollForConversion(&hadc1, 100);
				data_sensors.soil_moisture_2=HAL_ADC_GetValue(&hadc1);
				data_sensors.soil_moisture_2=map(data_sensors.soil_moisture_2, 1300,2700, 0,100);
				HAL_ADC_PollForConversion(&hadc1, 100);
				data_sensors.radiacion= HAL_ADC_GetValue(&hadc1);
				data_sensors.radiacion= map(data_sensors.radiacion, 1000, 1800, 0, 11);
				HAL_ADC_Stop(&hadc1);
				event_alarms.sensors_data=data_sensors;
				xQueueSend(queue_data,&data_sensors,0);
				xQueueSend(queue_alarms,&event_alarms,0);
				break;
			default:
				break;
		}
	}
}


