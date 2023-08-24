/*
 * task_raise_server.h
 *
 *  Created on: 7 feb. 2023
 *      Author: ferna
 */

#ifndef INC_TASK_MANAGEMENT_CONECTION_SERVER_MQTT_H_
#define INC_TASK_MANAGEMENT_CONECTION_SERVER_MQTT_H_

#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "aht10.h"
#include "callback_functions.h"
#include "adc.h"
#include "task_data_acquisition.h"
#include "driver_bg96.h"

typedef enum
{
	UP_SET_PARAMETER_CONTEXT_TCP,
	UP_ACTIVATE_CONTEXT_PDP,
	UP_OPEN_CLIENT_MQTT,
	UP_CONNECT_BROKER_MQTT,
	UP_ERROR_CONECTION,
	RESET_MODEM,
	STATUS,
}en_up_conection;

typedef enum
{
	DOWN_CLOSE_BROKE_MQTT,
	DOWN_DISCONNECT_BROKER_MQTT,
	DOWN_DESACTIVATE_MQTT,
	DOWN_ERROR_CONECTION,
}en_down_conection;

typedef enum
{
	UP_CONECTION,
	DOWN_CONECTION,
	SEND_DATA_MQTT,
}en_event_conection;

typedef struct
{
	en_event_conection event_conection;

}st_event_conection;

struct st_config_mqtt_server
{
	char topic[30];
};

void task_raise_server(void *p_parameter);
void task_management_conection_server_mqtt(void *p_parameter);
em_bg96_error_handling write_data(const char *command, const char *request, char *buffer, uint32_t time);
void reset_modem(void);
void on_modem(void);
void off_modem(void);

#endif /* INC_TASK_MANAGEMENT_CONECTION_SERVER_MQTT_H_ */
