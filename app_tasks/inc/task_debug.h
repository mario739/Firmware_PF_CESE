/**
 * @file task_debug.h
 * @author Mario Aguilar Montoya (fernandoaguilar731010@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef INC_TASK_DEBUG_H_
#define INC_TASK_DEBUG_H_

#include "app.h"
#include "cmsis_os.h"
#include "main.h"
#include "aht10.h"
#include "driver_bg96.h"
#include "callback_functions.h"
#include "adc.h"
#include "task_data_acquisition.h"

void task_debug(void *p_parameter);

#endif /* INC_TASK_DEBUG_H_ */
