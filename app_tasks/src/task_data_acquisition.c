

#include "task_data_acquisition.h"
#include "adc.h"

extern ADC_HandleTypeDef hadc1;
extern xQueueHandle queue_data;
extern aht10_config_t aht_config;

/**
 * @brief tarea que adquiere los datos de los sensores
 * 
 * @param p_parameter no utilizado 
 */


static uint32_t map(uint32_t value, uint32_t inputMin, uint32_t inputMax, uint32_t outputMin, uint32_t outputMax)
{
  return ((((value - inputMin) * (outputMax - outputMin)) / (inputMax - inputMin)) + outputMin);
}

void task_data_acquisition(void *p_parameter)
{
	struct st_data_sensors data_sensors={.ambient_humidity=20,.ambient_temperature=20,.batery=30,.radiacion=10,.soil_moisture_1=20};
	while (1)
	{
		aht10_get_humedity(&aht_config,&data_sensors.ambient_humidity);
		aht10_get_temperature(&aht_config,&data_sensors.ambient_temperature);
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.soil_moisture_1= map(HAL_ADC_GetValue(&hadc1), 2700, 4001, 0, 100);
		data_sensors.soil_moisture_1= 100 - data_sensors.soil_moisture_1;
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.soil_moisture_2= map(HAL_ADC_GetValue(&hadc1), 2700, 4001, 0, 100);
		data_sensors.soil_moisture_2= 100 - data_sensors.soil_moisture_2;
		HAL_ADC_PollForConversion(&hadc1, 100);
		data_sensors.radiacion= map(HAL_ADC_GetValue(&hadc1), 1000, 2500, 0, 15);
		HAL_ADC_Stop(&hadc1);
		xQueueSend(queue_data,&data_sensors,portMAX_DELAY);
	}
}

