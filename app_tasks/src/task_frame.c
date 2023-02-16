/*
 * task_frame.c
 *
 *  Created on: 16 feb. 2023
 *      Author: ferna
 */


/*static void task_data_concatenate(void *p_parameter)
{
	struct st_data_sensors data_sensors2;
	static char data2[150];
	while(1)
	{
		xQueueReceive(queue_data, &data_sensors2,portMAX_DELAY);
		sprintf(data2,"\"bateria\":%u,\"humedad\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperatura_ambiente\":%u}",data_sensors2.batery,
		data_sensors2.soil_moisture,data_sensors2.ambient_humidity,data_sensors2.radiacion,data_sensors2.ambient_temperature);
		xQueueSend(queue_trama,data2,200);
		vTaskDelay(20);
	}
}*/
