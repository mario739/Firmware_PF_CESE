/**
 * @file driver_bg96.h
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef DRIVER_BG96_H
#define DRIVER_BG96_H

/*private includes*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"



typedef enum
{
  FT_BG96_OK=0,
  FT_BG96_ERROR=1,
  FT_BG96_BUSY=2,
  FT_BG96_TIMEOUT=3,
} em_bg96_error_handling;

/**
 * @brief Puntero a funcion para enviar datos 
 * 
 * 
 */
typedef em_bg96_error_handling(*pf_send_data)( const char*,const char*,char*,uint32_t);

typedef void(*pf_reset_modem)(void);
/**
 * @brief Enumerador para la maquina des estados 
 * del conecion mqtt
 * 
 */
typedef enum
{
    OPEN_CONNECTION_MQTT,
    CONNECT_BROKER_MQTT,
    PUB_MQTT,
    DISCONNECT_BROKER_MQTT,
    CLOSE_BROKEN_MQTT,
    ERROR1,
}em_states_send_data;


typedef enum{
	SERVER_MQTT_UP,
	SERVER_MQTT_DOWN,
}em_state_server_mqtt_conection;

typedef enum
{
    ON,
    OFF,
}em_status_modem;

typedef struct 
{
    uint8_t context_id;            //ID del contexto rango 1-16
    uint8_t context_type;          //Tipo de protocolo 1=IPV4,2=IPV6
    char *tcp_apn;                 //Nombre del punto de acceso
    char *tcp_username;           //Nombre de usuario
    char *tcp_password;            //Password 
    uint8_t method_authentication; //Metodo de autentificacion 
}st_config_context_tcp;

typedef struct 
{
    uint8_t identifier_socket_mqtt; //Numero del socket configurado 1-5
    uint8_t quality_service;        //Calidad de servicio del socket MQTT 
    char *host_name;
    char *mqtt_username;
    uint32_t port;
    char *mqtt_password;
    char *mqtt_client_id;

}st_config_parameters_mqtt;

typedef struct 
{
    uint8_t mark[10];
    uint8_t model[10];
    uint8_t revision[50];
}st_info_product;

typedef struct 
{
    em_status_modem  status_modem;
    em_state_server_mqtt_conection status_mqtt_server;
    pf_send_data    send_data_device;
    pf_reset_modem  f_reset_modem; 
    st_config_parameters_mqtt self_mqtt;
    st_config_context_tcp self_tcp;
    st_info_product info_product;
    uint8_t           last_error;
    char buffer_resp [100];
    char *current_cmd;
    em_bg96_error_handling ft_resp;
}st_bg96_config;



em_bg96_error_handling init_driver(st_bg96_config *self,pf_send_data ft_send_data_device, pf_reset_modem ft_reset_modem);
em_bg96_error_handling turn_off_bg96(st_bg96_config *self);

em_bg96_error_handling get_status_modem(st_bg96_config *self);
em_bg96_error_handling get_status_sim(st_bg96_config *self);
em_bg96_error_handling get_information_product(st_bg96_config *self);
em_bg96_error_handling get_status_pdp_context(st_bg96_config *self);

em_bg96_error_handling set_mode_echo(st_bg96_config *self,uint8_t mode);
em_bg96_error_handling set_format_response(st_bg96_config *self,uint8_t mode);
em_bg96_error_handling set_format_error(st_bg96_config *self,uint8_t mode);
em_bg96_error_handling set_sms_format(st_bg96_config *self,uint8_t mode);

em_bg96_error_handling set_parameter_context_tcp(st_bg96_config *self);
em_bg96_error_handling activate_context_pdp(st_bg96_config *self);
em_bg96_error_handling desactivate_context_pdp(st_bg96_config *self);
em_bg96_error_handling send_sms_bg96(st_bg96_config *self,char*number,char*message);
em_bg96_error_handling set_parameters_mqtt(st_bg96_config *self);
em_bg96_error_handling open_client_mqtt(st_bg96_config *self);
em_bg96_error_handling connect_server_mqtt(st_bg96_config *self);
em_bg96_error_handling publish_message(st_bg96_config *self,char *topic,char *data);
em_bg96_error_handling close_client_mqtt(st_bg96_config *self);
em_bg96_error_handling disconnect_server_mqtt(st_bg96_config *self);

em_bg96_error_handling send_data_mqtt(st_bg96_config *self,char *topic,char *data);
#endif
