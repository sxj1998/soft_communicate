/*
 * @Description:
 * @Author: shexingju && rick.she@suntek-group.com
 * @Date: 2023-08-10 01:51:45
 * @LastEditors: shexingju && rick.she@suntek-group.com
 * @LastEditTime: 2023-08-10 01:52:54
 * @FilePath: soft_iic_M.h
 * Copyright 2023 shexingju, All Rights Reserved.
 * 2023-08-10 01:51:45
 */
/**
 *******************************************************************************
 * @file        : swI2c.h
 * @author      : unique
 * @brief       : None
 * @attention   : None
 * @date        : 2023/5/28
 *******************************************************************************
 */

#ifndef SW_I2C_H
#define SW_I2C_H
#ifdef __cpluscplus
extern "C"
{
#endif
#include "stdint.h"
#include "interfaceMapping.h"

    typedef struct
    {
        uint8_t dummy;
    } swI2cHandlerTypeDef;

#define I2C_WAIT_TIMEOUT (uint16_t)(100)
#define I2C_DELAY (uint8_t)(50)

#define I2C_N_ACK (0U)
#define I2C_ACK (1U)

#define SW_I2C_SCL_LOW gpio_bit_write(SW_I2C_DRIVER_SCL_PORT, SW_I2C_DRIVER_SCL_PIN, 0)
#define SW_I2C_SCL_HIGH gpio_bit_write(SW_I2C_DRIVER_SCL_PORT, SW_I2C_DRIVER_SCL_PIN, 1)
#define SW_I2C_SDA_LOW gpio_bit_write(SW_I2C_DRIVER_SDA_PORT, SW_I2C_DRIVER_SDA_PIN, 0)
#define SW_I2C_SDA_HIGH gpio_bit_write(SW_I2C_DRIVER_SDA_PORT, SW_I2C_DRIVER_SDA_PIN, 1)

#define I2C_SDA_STATUS gpio_input_bit_get(SW_I2C_DRIVER_SDA_PORT, SW_I2C_DRIVER_SDA_PIN)

#define swI2cDelayUs(t) for (int _counter = 0; _counter < t; _counter++)

#define I2C_USE_7BIT_ADDR

    void swI2cGpioInit(void);
    void swI2cStart(void);
    void swI2cStop(void);
    void swI2cSendAck(void);
    void swI2cSendNAck(void);
    uint8_t swI2cWaitAck(void);
    void swI2cWriteByte(uint8_t aByte);
    uint8_t swI2cReadByte(uint8_t Ack);
    int8_t swI2cSendNBytes(uint8_t I2C_Address, uint8_t *Data, uint16_t Datalen);
    int8_t swI2cReadNBytes(uint8_t I2C_Address, uint8_t *Data, uint16_t Datalen);

#ifdef __cpluscplus
}
#endif
#endif
