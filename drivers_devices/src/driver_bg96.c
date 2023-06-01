/**
 * @file driver_bg96.c
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */
 
#include "driver_bg96.h"




//static const char RS_BG96_OK[]="OK\r\n";
static const char RS_BG96_SIGNAL[]=">";
static const char RS_BG96_CERO[]="0\r\n";
static const char CMD_BG96_STATUS_MODEM[]="AT\r";
static const char CMD_BG96_STATUS_SIM[]="AT+CPIN?\r";
static const char CMD_BG96_GET_INFO_PRODUCT[]="ATI\r";
static const char CMD_BG96_MODE_ECHO_OFF[]="ATE0\r";
static const char CMD_BG96_MODE_ECHO_ON[]="ATE1\r";
static const char CMD_BG96_MODE_RESPONSE_LONG[]="ATV1\r";
static const char CMD_BG96_MODE_RESPONSE_SHORT[]="ATV0\r";
static const char CMD_BG96_STATUS_PDP_CONTEXT[]="AT+QIACT?\r";


em_bg96_error_handling init_driver(st_bg96_config *self,pf_send_data ft_send_data_device,pf_reset_modem ft_reset_modem)
{
    if (ft_send_data_device!=NULL) {
    	self->send_data_device=ft_send_data_device;
	}
    if (ft_reset_modem!=NULL) {
        self->f_reset_modem=ft_reset_modem;
	}
    self->ft_resp=FT_BG96_OK;
	self->last_error=BG96_NO_ERROR;
	self->self_tcp.context_id=1;
	self->self_tcp.context_type=1;
	self->self_tcp.method_authentication=1;
	self->self_tcp.tcp_apn="4g.entel";
	self->self_tcp.tcp_password="";
	self->self_tcp.tcp_username="";

	self->self_mqtt.identifier_socket_mqtt=0;
	self->self_mqtt.quality_service=0;
	self->self_mqtt.host_name="\"industrial.api.ubidots.com\"";
	self->self_mqtt.port=1883;
	self->self_mqtt.mqtt_client_id="123a56cb9";
	self->self_mqtt.mqtt_username="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL";
	self->self_mqtt.mqtt_password="BBFF-YymzfOGNgPBLoxxhddQT99r9Wq77rL";

	self->status_mqtt_server=SERVER_MQTT_DOWN;

    return self->ft_resp;
}
em_bg96_error_handling get_status_modem(st_bg96_config* self)
{
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device(CMD_BG96_STATUS_MODEM,RS_BG96_OK,self->buffer_resp,1000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_STATUS_MODEM;
    }
    return self->ft_resp;
}
em_bg96_error_handling get_status_sim(st_bg96_config *self)
{   
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device(CMD_BG96_STATUS_SIM,RS_BG96_OK,self->buffer_resp,5000);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_STATUS_SIM;
    }
    return self->ft_resp;
}
em_bg96_error_handling get_information_product(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device(CMD_BG96_GET_INFO_PRODUCT,RS_BG96_OK,self->buffer_resp,300);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_INFO_PRODUCTO;
    }
    return self->ft_resp;
    
}
em_bg96_error_handling get_status_pdp_context(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device(CMD_BG96_STATUS_PDP_CONTEXT,RS_BG96_OK,self->buffer_resp,1000);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_PDP_CONTEXT;
    }
    return self->ft_resp;
}


/**
 * @brief Set the mode echo object
 * 
 * @param self puntero de la structura del objeto del modulo 
 * @param mode 1:enable 0:unenable 
 * @return em_bg96_error_handling 
 */

em_bg96_error_handling set_mode_echo(st_bg96_config *self,uint8_t mode)
{
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device( mode==0? CMD_BG96_MODE_ECHO_OFF:CMD_BG96_MODE_ECHO_ON,RS_BG96_OK,self->buffer_resp,300);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_SET_MODE_ECHO;   
    }
    return self->ft_resp;
}


/**
 * @brief Set the format response object
 * 
 * @param self 
 * @param mode 0:Short 1:Long
 * @return em_bg96_error_handling 
 */

em_bg96_error_handling set_format_response(st_bg96_config *self,uint8_t mode)
{
    self->ft_resp=FT_BG96_OK;
    self->ft_resp=self->send_data_device(mode==0? CMD_BG96_MODE_RESPONSE_SHORT:CMD_BG96_MODE_RESPONSE_LONG,RS_BG96_OK,self->buffer_resp,300);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_SET_FORMAT_RESPONSE;
    }
    return self->ft_resp;
}


/**
 * @brief Set the format error object
 * 
 * @param self 
 * @param mode 0:Disable 1:Numeric 2:Text
 * @return em_bg96_error_handling 
 */
em_bg96_error_handling set_format_error(st_bg96_config *self,uint8_t mode)
{
    self->ft_resp=FT_BG96_OK;
    char cmd[20];
    sprintf(cmd,"AT+CMEE=%u\r",mode);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,300);
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_SET_MODE_ERROR;
    }
    return self->ft_resp;
}


/**
 * @brief Set the sms format object
 * 
 * @param self 
 * @param mode 0:PDU 1:TEXT
 * @return em_bg96_error_handling 
 */
em_bg96_error_handling set_sms_format(st_bg96_config *self,uint8_t mode)
{
    self->ft_resp=FT_BG96_OK;
    char cmd[12]="AT+CMGF=";
    strcat(cmd,(mode>0)?"1":"0");
    strcat(cmd,"\r");
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,300);   
    if (FT_BG96_OK!=self->ft_resp)
    {
        self->last_error=BG96_ERROR_SET_SMS_FORMAT;
    }
    return self->ft_resp;
}

em_bg96_error_handling send_sms_bg96(st_bg96_config *self,char*number,char*message)
{
    self->ft_resp=FT_BG96_OK;
    char buffer_message[256];
    char buffer_number[20];
    sprintf(buffer_number,"AT+CMGS=\"%s\"\r",number);
    sprintf(buffer_message,"%s\x1a\r",message);

    self->ft_resp=self->send_data_device(buffer_number,RS_BG96_SIGNAL,self->buffer_resp,12000);
    if (FT_BG96_OK==self->ft_resp)
    {
        self->ft_resp=self->send_data_device(buffer_message,RS_BG96_OK,self->buffer_resp,12000);
        if (FT_BG96_OK!=self->ft_resp)
        {
            self->last_error=BG96_ERROR_SEND_SMS;
        }
    }
    return self->ft_resp;
}

em_bg96_error_handling set_parameter_context_tcp(st_bg96_config *self)
{   
    self->ft_resp=FT_BG96_OK;
    char cmd[100];
    sprintf(cmd,"AT+QICSGP=%u,%u,\"%s\",\"%s\",\"%s\",%u\r",self->self_tcp.context_id,self->self_tcp.context_type,self->self_tcp.tcp_apn,self->self_tcp.tcp_username,self->self_tcp.tcp_password,self->self_tcp.method_authentication);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,3000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_SET_PARAMETER_CONTEXT_TCP;
    }
    return self->ft_resp;
}

/**
 * @brief 
 * 
 * @param self 
 * @return em_bg96_error_handling 
 */
em_bg96_error_handling activate_context_pdp(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_OK;
    char cmd[30];
    sprintf(cmd,"AT+QIACT=%u\r",self->self_tcp.context_id);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,15000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_ACTIVATE_CONTEXT_PDP;
    }
    return self->ft_resp;
}

/**
 * @brief 
 * 
 * @param self 
 * @return em_bg96_error_handling 
 */
em_bg96_error_handling desactivate_context_pdp(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_OK;
    char cmd[15];
    sprintf(cmd,"AT+QIDEACT=%u\r",self->self_tcp.context_id);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,40000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_DESACTIVATE_CONTEXT_PDP;
    }
    return self->ft_resp;
}
em_bg96_error_handling set_parameters_mqtt(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd_pdpid[30];
    sprintf(cmd_pdpid,"AT+QMTCFG=\"pdpid\",%u,%u\r",self->self_mqtt.identifier_socket_mqtt,self->self_tcp.context_id);
    self->ft_resp=self->send_data_device(cmd_pdpid,RS_BG96_OK,self->buffer_resp,300);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_SET_PARAMETER_MQTT;
    }
    return self->ft_resp;
}

em_bg96_error_handling open_client_mqtt(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[100];
    sprintf(cmd,"AT+QMTOPEN=%u,%s,%lu\r",self->self_mqtt.identifier_socket_mqtt,self->self_mqtt.host_name,self->self_mqtt.port);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_CERO,self->buffer_resp,75000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_OPEN_CLIENT_MQTT;

    }

    return self->ft_resp;
}
em_bg96_error_handling close_client_mqtt(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[50];
    sprintf(cmd,"AT+QMTCLOSE=%u\r",self->self_mqtt.identifier_socket_mqtt);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,3000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_CLOSE_CLIENT_MQTT;
    }
    return self->ft_resp;
}

em_bg96_error_handling connect_server_mqtt(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[150]={0};
    sprintf(cmd,"AT+QMTCONN=%u,\"%s\",\"%s\",\"%s\"\r",self->self_mqtt.identifier_socket_mqtt,self->self_mqtt.mqtt_client_id,self->self_mqtt.mqtt_username,self->self_mqtt.mqtt_password);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_CERO,self->buffer_resp,7000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_CONNECT_SERVER_MQTT;
    }
   return self->ft_resp;
}

em_bg96_error_handling disconnect_server_mqtt(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[50]={0};
    sprintf(cmd,"AT+QMTDISC=%u\r",self->self_mqtt.identifier_socket_mqtt);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,5000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_DISCONNECT_SERVER_MQTT;
    }
    return self->ft_resp;
}


em_bg96_error_handling publish_message(st_bg96_config *self,char *topic,char *data)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[50]={0};
    char buffer_data[220]={0};
    sprintf(buffer_data,"%s\x1a\r",data);
    sprintf(cmd,"AT+QMTPUB=%u,0,0,0,\"%s\"\r",self->self_mqtt.identifier_socket_mqtt,topic);
    self->ft_resp=self->send_data_device(cmd,RS_BG96_SIGNAL,self->buffer_resp,3000);
    if (FT_BG96_OK==self->ft_resp)
    {
        self->ft_resp=self->send_data_device(buffer_data,RS_BG96_CERO,self->buffer_resp,15000);
        if (self->ft_resp!=FT_BG96_OK)
        {
            self->last_error=BG96_ERROR_PUBLISH_MESSAGE;
        }   
    }
    else self->last_error=BG96_ERROR_PUBLISH_MESSAGE;
    return self->ft_resp;
}

em_bg96_error_handling turn_off_bg96(st_bg96_config *self)
{
    self->ft_resp=FT_BG96_ERROR;
    char cmd[15]="AT+QPOWD\r";
    self->ft_resp=self->send_data_device(cmd,RS_BG96_OK,self->buffer_resp,1000);
    if (self->ft_resp!=FT_BG96_OK)
    {
        self->last_error=BG96_ERROR_TURN_OFF;
    }
    return self->ft_resp;

}

em_bg96_error_handling send_data_mqtt(st_bg96_config *self,char *topic,char *data)
{
    self->ft_resp=FT_BG96_OK;
    em_states_send_data states_send_data_mqtt=PUB_MQTT;
    uint8_t flag_machine=1;
    while (flag_machine==1)
    {
        switch (states_send_data_mqtt)
        {
        	case PUB_MQTT:
				if (publish_message(self,topic,data)==FT_BG96_OK)
				{
					flag_machine=0;
				}
				else
				{
					states_send_data_mqtt=ERROR1;
				}
			break;
        case ERROR1:
        	flag_machine=0;
            self->ft_resp=FT_BG96_ERROR;
            break;
        default:
            flag_machine=0;
            break;
        }   
    }
    return self->ft_resp;   
}
