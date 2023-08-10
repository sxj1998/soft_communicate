/*
 * @Description:
 * @Author: shexingju && rick.she@suntek-group.com
 * @Date: 2023-08-10 01:53:12
 * @LastEditors: shexingju && rick.she@suntek-group.com
 * @LastEditTime: 2023-08-10 01:54:30
 * @FilePath: soft_iic_S.h
 * Copyright 2023 shexingju, All Rights Reserved.
 * 2023-08-10 01:53:12
 */
/**
 *******************************************************************************
 * @file        : swI2c.h
 * @author      : unique
 * @brief       : None
 * @attention   : None
 * @date        : 2023/3/10
 *******************************************************************************
 */

#ifndef SW_I2C_H
#define SW_I2C_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "interfaceMapping.h"

    typedef enum
    {
        I2C_STA_IDLE = 0,
        I2C_STA_START,
        I2C_STA_DATA,
        I2C_STA_ACK,
        I2C_STA_NACK,
        I2C_STA_STOP,
    } i2cStateTypeDef;

    typedef enum
    {
        I2C_EDGE_NONE = 0,
        I2C_SCL_RISING = 1,
        I2C_SCL_FALLING,
        I2C_SDA_RISING,
        I2C_SDA_FALLING,
    } i2cPinEdgeTypeDef;

#define SW_SLAVE_ADDR (0x01 << 1)

#define SET_SDA_DIR(dir) GPIO_SetPinDir(SW_I2C_GPIO_SDA, dir)
#define GET_SCL_STATE lmGpioReadPin(SW_I2C_GPIO_SCL)
#define GET_SDA_STATE lmGpioReadPin(SW_I2C_GPIO_SDA)

#define I2C_READ 1
#define I2C_WRITE 0

#define GPIO_DIR_IN GPIO_INPUT
#define GPIO_DIR_OUT GPIO_OUTPUT

#define CLR_SDA_PIN() lmGpioResetPin(SW_I2C_GPIO_SDA)
#define SET_SDA_PIN() lmGpioSetPin(SW_I2C_GPIO_SDA)

    typedef struct
    {
        uint8_t State;
        uint8_t Rw;
        uint8_t SclFallCnt;
        uint8_t Flag;
        uint8_t RxBuf[256];
        uint8_t TxBuf[256];
        uint8_t RxIdx;
        uint8_t TxIdx;

        uint8_t scl_state;
        uint8_t sda_state;

        uint8_t set_tx_data_enable;
    } swI2cSlaveTypeDef;

    void swI2cProc(swI2cSlaveTypeDef *handler, i2cPinEdgeTypeDef edge);
    void swI2cReset(swI2cSlaveTypeDef *handler);
    int8_t swI2cSetTxData(swI2cSlaveTypeDef *handler, uint8_t *data_p, uint8_t data_size);

#ifdef __cplusplus
}
#endif
#endif
