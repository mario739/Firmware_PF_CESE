

#include "task_raise_server.h"

uint8_t  rx_tem;
uint8_t rx_buffer[300];
uint16_t rx_index=0;

enum en_raise_server
{
	STATUS_MODEM,
	STATUS_SIM,
	SET_PARAMETER_CONTEXT_TCP,
	ACTIVATE_CONTEXT_PDP,
	SEND_DATA_MQTT,
	DESACTIVATE_MQTT,
	SEND_ERROR,
};

void task_raise_server(void *p_parameter)
{
	struct st_data_sensors data_sensors2;
	struct st_config_mqtt_server config_mqtt_server={.topic="/v1.6/devices/monitoreo_iot"};
	char data[200];
	enum en_raise_server raise_server=STATUS_MODEM;
	while(1)
	{
		switch (raise_server) {
			case STATUS_MODEM:
				if (get_status_modem(&bg96_config)==FT_BG96_OK) {
					raise_server=STATUS_SIM;
				}
				else {
					raise_server=SEND_ERROR;
				}
				break;
			case STATUS_SIM:
				if (get_status_sim(&bg96_config)==FT_BG96_OK) {
					raise_server=SET_PARAMETER_CONTEXT_TCP;
				}
				else {
					raise_server=SEND_ERROR;
				}
				break;
			case SET_PARAMETER_CONTEXT_TCP:
				if (set_parameter_context_tcp(&bg96_config)==FT_BG96_OK) {
					raise_server=ACTIVATE_CONTEXT_PDP;
				}
				else {
					raise_server=SEND_ERROR;
				}
				break;
			case ACTIVATE_CONTEXT_PDP:
				if (activate_context_pdp(&bg96_config)==FT_BG96_OK) {
					xQueueReceive(queue_data,&data_sensors2,300);
					sprintf(data,"{\"bateria\":%u,\"humedad\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperatura_ambiente\":%u}",data_sensors2.batery,
					data_sensors2.soil_moisture_1,data_sensors2.ambient_humidity,data_sensors2.radiacion,data_sensors2.ambient_temperature);
					raise_server=SEND_DATA_MQTT;
				}
				else {
					raise_server=DESACTIVATE_MQTT;
				}
				break;
			case SEND_DATA_MQTT:
				if (send_data_mqtt(&bg96_config,config_mqtt_server.topic,data)==FT_BG96_OK) {
					raise_server=STATUS_MODEM;
				} else {
					raise_server=DESACTIVATE_MQTT;
				}
				break;
			case DESACTIVATE_MQTT:
				if (desactivate_context_pdp(&bg96_config)==FT_BG96_OK) {
					raise_server=STATUS_MODEM;
				} else {
					raise_server=SEND_ERROR;
				}
				break;
			case SEND_ERROR:
				raise_server=STATUS_MODEM;
				break;
			default:
				break;
		}
		vTaskDelay(100);
	}
}
em_bg96_error_handling write_data(char *command, char *request, char *buffer, uint32_t time)
{
	HAL_UART_Transmit(&huart1, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Transmit(&huart2, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	char* res;
	for (uint32_t i = 0; i < time; ++i)
	{
		vTaskDelay(1);
		res=strstr((char*)rx_buffer,request);
		if (res!= NULL)
		{
			HAL_UART_Transmit(&huart2, rx_buffer,strlen((const char *)rx_buffer),200);
			memset((char*)rx_buffer, 0, sizeof(rx_buffer));
			rx_index=0;
			return FT_BG96_OK;
		}
	}
	rx_index=0;
	HAL_UART_Transmit(&huart2, rx_buffer,strlen((const char *)rx_buffer),200);
	memset((char*)rx_buffer, 0, sizeof(rx_buffer));
	return FT_BG96_TIMEOUT;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart==&huart1)
	{
		rx_buffer[rx_index]=(char)rx_tem;
		rx_index++;
		rx_tem=0;
		HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	}
}
