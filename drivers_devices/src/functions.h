/**
 * @file string_management.h
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef STRING_MANAGEMENT
#define STRING_MANAGEMENT

#include "driver_bg96.h"

em_bg96_error_handling send_data(const char *cmd,const char*expect,char* resp,uint32_t timeout);
void test_pf_reset_modem(void);
#endif