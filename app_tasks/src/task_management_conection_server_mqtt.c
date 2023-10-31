

#include <task_management_conection_server_mqtt.h>

volatile uint8_t  rx_tem;
volatile uint8_t  rx_buffer[300];
volatile uint16_t rx_index=0;

extern xQueueHandle queue_server_mqtt;
extern SemaphoreHandle_t semaphore_loop;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern xQueueHandle queue_data;
extern st_bg96_config bg96_config;

void reset_modem(void)
{
	off_modem();
	on_modem();
}

void on_modem(void)
{
	HAL_GPIO_WritePin(GPIOA,output_reset_Pin, GPIO_PIN_SET);
	vTaskDelay(700);
	HAL_GPIO_WritePin(GPIOA,output_reset_Pin, GPIO_PIN_RESET);
	vTaskDelay(4500);
	vTaskDelay(20000);
}

void off_modem(void)
{
	HAL_GPIO_WritePin(GPIOA,output_reset_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,output_reset_Pin, GPIO_PIN_SET);
	vTaskDelay(700);
	HAL_GPIO_WritePin(GPIOA,output_reset_Pin, GPIO_PIN_RESET);
	vTaskDelay(4500);
}

void task_management_conection_server_mqtt(void *p_parameter)
{
	st_event_conection p_event_conection;
	st_data_sensors data_sensor;
	st_config_mqtt_server config_mqtt_server={.topic="v1/devices/me/telemetry",.down_conection=DOWN_CLOSE_BROKE_MQTT,.up_conection=RESET_MODEM};
	char data[200];
	uint8_t flag=1;
	uint8_t cont_status=0;

	while(1)
	{
		xQueueReceive(queue_server_mqtt,&p_event_conection,portMAX_DELAY);
		flag=1;
		while(flag)
		{
			switch(p_event_conection.event_conection){
				case UP_CONECTION:
					switch (config_mqtt_server.up_conection) {
						case RESET_MODEM:
							if (bg96_config.status_modem==OFF)
							{
								if (get_status_modem(&bg96_config)==FT_BG96_OK)
								{
									reset_modem();
									config_mqtt_server.up_conection=STATUS;
									cont_status=0;
								}
								else {
									if (cont_status==2) {
										on_modem();
										config_mqtt_server.up_conection=STATUS;
										cont_status=0;
									}
									else {
										cont_status++;
									}
								}
							}
							else if(bg96_config.status_modem==ON) {
								reset_modem();
						     	config_mqtt_server.up_conection=STATUS;
								bg96_config.status_modem=ON;
							}
							break;
						case STATUS:
							if (get_status_modem(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.up_conection=UP_SET_PARAMETER_CONTEXT_TCP;
								cont_status=0;
							}
							else {
								if (cont_status==2) {
									config_mqtt_server.up_conection=UP_ERROR_CONECTION;
									cont_status=0;
								}
								else
								{
									cont_status++;
								}
							}
							break;
						case UP_SET_PARAMETER_CONTEXT_TCP:
							if (set_parameter_context_tcp(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.up_conection=UP_ACTIVATE_CONTEXT_PDP;
							}
							else {
								config_mqtt_server.up_conection=UP_ERROR_CONECTION;
							}
							break;
						case UP_ACTIVATE_CONTEXT_PDP:
							if (activate_context_pdp(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.up_conection=UP_OPEN_CLIENT_MQTT;
							}
							else {
								config_mqtt_server.up_conection=UP_ERROR_CONECTION;
							}
							break;
						case UP_OPEN_CLIENT_MQTT:
							if (open_client_mqtt(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.up_conection=UP_CONNECT_BROKER_MQTT;
							}
							else{
								config_mqtt_server.up_conection=UP_ERROR_CONECTION;
							}
							break;
						case UP_CONNECT_BROKER_MQTT:
							if (connect_server_mqtt(&bg96_config)==FT_BG96_OK) {
								bg96_config.status_mqtt_server=SERVER_MQTT_UP;
								HAL_GPIO_WritePin(output_signal_GPIO_Port,output_signal_Pin, GPIO_PIN_SET);
								config_mqtt_server.up_conection=RESET_MODEM;
								flag=0;
								xSemaphoreGive(semaphore_loop);
							}
							else{
								config_mqtt_server.up_conection=UP_ERROR_CONECTION;
							}
							break;
						case UP_ERROR_CONECTION:
							config_mqtt_server.up_conection=RESET_MODEM;
							flag=0;
							xSemaphoreGive(semaphore_loop);
							break;
						default:
							break;
					}
					break;
				case DOWN_CONECTION:
					switch (config_mqtt_server.down_conection) {
						case DOWN_CLOSE_BROKE_MQTT:
							if (close_client_mqtt(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.down_conection=DOWN_DISCONNECT_BROKER_MQTT;
							}
							else{
								config_mqtt_server.down_conection=DOWN_DESACTIVATE_MQTT;
							}
							break;
						case DOWN_DISCONNECT_BROKER_MQTT:
							if (disconnect_server_mqtt(&bg96_config)==FT_BG96_OK) {
								config_mqtt_server.down_conection=DOWN_DESACTIVATE_MQTT;
							}
							else{
								config_mqtt_server.down_conection=DOWN_ERROR_CONECTION;
							}
							break;
						case DOWN_DESACTIVATE_MQTT:
							if (desactivate_context_pdp(&bg96_config)==FT_BG96_OK) {
								bg96_config.status_mqtt_server=SERVER_MQTT_DOWN;
								HAL_GPIO_WritePin(output_signal_GPIO_Port,output_signal_Pin, GPIO_PIN_RESET);
								config_mqtt_server.down_conection=DOWN_CLOSE_BROKE_MQTT;
								flag=0;
								off_modem();
								bg96_config.status_modem=OFF;
								xSemaphoreGive(semaphore_loop);
							}
							else {
								config_mqtt_server.down_conection=DOWN_ERROR_CONECTION;
							}
							break;
						case DOWN_ERROR_CONECTION:
							config_mqtt_server.down_conection=DOWN_CLOSE_BROKE_MQTT;
							HAL_GPIO_WritePin(output_signal_GPIO_Port,output_signal_Pin, GPIO_PIN_RESET);
							flag=0;
							off_modem();
							bg96_config.status_modem=OFF;
							xSemaphoreGive(semaphore_loop);
							break;
						default:
							break;
					}
					break;
				case SEND_DATA_MQTT:
						xQueueReceive(queue_data,&data_sensor,portMAX_DELAY);
						sprintf(data,"{\"humedad_suelo\":%u,\"humedad_ambiente\":%u,\"radiacion\":%u,\"temperature_ambiente\":%u,\"position\":{\"latitude\":%f, \"longitude\":%f}}",
						data_sensor.soil_moisture_1,data_sensor.ambient_humidity,data_sensor.radiacion,data_sensor.ambient_temperature,data_sensor.latitude,data_sensor.longitude);
						publish_message(&bg96_config, config_mqtt_server.topic, data);
						flag=0;
						xSemaphoreGive(semaphore_loop);
					break;
				default:
					break;
			}
		}
	}
}

em_bg96_error_handling write_data(const char *command, const char *request, char *buffer, uint32_t time)
{
	char* res;
	uint32_t i;

	HAL_UART_Transmit(&huart2, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Transmit(&huart1, (const uint8_t*)command, strlen(command),50);
	HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	for (i = 0; i < time; ++i)
	{
		HAL_Delay(1);
		res=strstr((char*)rx_buffer,request);
		HAL_UART_Receive_IT(&huart1,&rx_tem,1);
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
		HAL_UART_Receive_IT(&huart1,&rx_tem,1);
	}
}

