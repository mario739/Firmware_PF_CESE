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
#include "driver_bg96.h"
#include "callback_functions.h"
#include "adc.h"
#include "task_data_acquisition.h"


/*enum en_raise_server
{
	STATUS_MODEM,
	STATUS_SIM,
	SET_PARAMETER_CONTEXT_TCP,
	ACTIVATE_CONTEXT_PDP,
	SEND_DATA_MQTT,
	DESACTIVATE_MQTT,
	SEND_ERROR,
};
*/
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


void reset_modem(void);
void task_raise_server(void *p_parameter);
void task_management_conection_server_mqtt(void *p_parameter);
em_bg96_error_handling write_data(const char *command, const char *request, char *buffer, uint32_t time);

#endif /* INC_TASK_MANAGEMENT_CONECTION_SERVER_MQTT_H_ */
