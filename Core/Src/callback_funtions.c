/*
 * callback_funtions.c
 *
 *  Created on: 29 sep. 2022
 *      Author: ferna
 */


#include "callback_functions.h"

extern I2C_HandleTypeDef hi2c1;

aht10_status_fnc write_I2C_STM32L432_port(uint8_t addr, uint8_t *buffer, uint8_t amount)
{
  HAL_I2C_Master_Transmit(&hi2c1, (addr << 1), buffer, amount, 500);
  return AHT10_OK;
}

aht10_status_fnc read_I2C_STM32L432_port(uint8_t addr, uint8_t *buffer, uint8_t amount)
{

  HAL_I2C_Master_Receive(&hi2c1, (addr << 1), buffer, amount, 500);
  return AHT10_OK;
}

void delay_STM32L432_port(uint8_t delay)
{
  vTaskDelay(delay);
}
