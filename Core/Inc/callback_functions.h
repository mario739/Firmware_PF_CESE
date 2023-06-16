/*
 * callback_functions.h
 *
 *  Created on: 29 sep. 2022
 *      Author: ferna
 */

#ifndef INC_CALLBACK_FUNCTIONS_H_
#define INC_CALLBACK_FUNCTIONS_H_

#include <string.h>
#include <stdio.h>
#include "i2c.h"
#include "aht10.h"

aht10_status_fnc write_I2C_STM32L432_port(uint8_t addr, uint8_t *buffer, uint8_t amount);

aht10_status_fnc read_I2C_STM32L432_port(uint8_t addr, uint8_t *buffer, uint8_t amount);

void delay_STM32L432_port(uint8_t delay);

#endif /* INC_CALLBACK_FUNCTIONS_H_ */
