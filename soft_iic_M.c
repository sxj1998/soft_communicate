/**
 *******************************************************************************
 * @file        : swI2c.h
 * @author      : unique
 * @brief       : None
 * @attention   : None
 * @date        : 2023/5/28
 *******************************************************************************
 */

#include "swI2c.h"

#ifdef USE_SW_I2C

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cGpioInit(void)
{
    rcu_periph_clock_enable(SW_I2C_DRIVER_SCL_RCU);
    rcu_periph_clock_enable(SW_I2C_DRIVER_SDA_RCU);

    /* configure GPIO */
    gpio_init(SW_I2C_DRIVER_SCL_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, SW_I2C_DRIVER_SCL_PIN);
    gpio_init(SW_I2C_DRIVER_SDA_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, SW_I2C_DRIVER_SDA_PIN);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cStart(void)
{
    SW_I2C_SDA_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SDA_LOW;
    swI2cDelayUs(I2C_DELAY);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cStop(void)
{
    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SDA_LOW;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SDA_HIGH;
    swI2cDelayUs(I2C_DELAY);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cSendAck(void)
{
    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SDA_LOW;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cSendNAck(void)
{
    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SDA_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
uint8_t swI2cWaitAck(void)
{
    uint16_t wait_time = 0;
    uint8_t ack_nack = 1;

    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);

    /*  release sda  */
    SW_I2C_SDA_HIGH;
    swI2cDelayUs(I2C_DELAY);

    SW_I2C_SCL_HIGH;
    swI2cDelayUs(I2C_DELAY);

    while (I2C_SDA_STATUS)
    {
        wait_time++;
        if (wait_time >= I2C_WAIT_TIMEOUT)
        {
            ack_nack = 0;
            break;
        }
        swI2cDelayUs(5);
    }

    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);

    return ack_nack;
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cWriteByte(uint8_t aByte)
{
    uint8_t i = 0;

    for (i = 0; i < 8; i++)
    {
        SW_I2C_SCL_LOW;
        swI2cDelayUs(I2C_DELAY);

        if (aByte & 0x80)
        {
            SW_I2C_SDA_HIGH;
        }
        else
        {
            SW_I2C_SDA_LOW;
        }
        swI2cDelayUs(I2C_DELAY);

        SW_I2C_SCL_HIGH;
        swI2cDelayUs(I2C_DELAY);

        aByte <<= 1;
    }

    SW_I2C_SCL_LOW;
    swI2cDelayUs(I2C_DELAY);
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
uint8_t swI2cReadByte(uint8_t Ack)
{
    uint8_t i = 0, aByte;

    SW_I2C_SDA_HIGH;

    for (i = 0; i < 8; i++)
    {
        aByte <<= 1;

        SW_I2C_SCL_LOW;
        swI2cDelayUs(I2C_DELAY);

        SW_I2C_SCL_HIGH;
        swI2cDelayUs(I2C_DELAY);

        if (I2C_SDA_STATUS)
        {
            aByte |= 0x01;
        }
    }
    SW_I2C_SCL_LOW;

    if (Ack)
    {
        swI2cSendAck();
    }
    else
    {
        swI2cSendNAck();
    }

    return aByte;
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
int8_t swI2cSendNBytes(uint8_t I2C_Address, uint8_t *Data, uint16_t Datalen)
{
    int8_t ret = 0;
    uint16_t i;

#ifdef I2C_USE_7BIT_ADDR
    I2C_Address <<= 1;
#endif

    swI2cStart();
    swI2cWriteByte((uint8_t)I2C_Address);

    if (!swI2cWaitAck())
    {
        swI2cStop();
        ret = -1;
    }
    else
    {
        for (i = 0; i < Datalen; i++)
        {
            swI2cWriteByte(Data[i]);
            if (!swI2cWaitAck())
            {
                swI2cStop();
                ret = -1;
                break;
            }
        }
    }
    swI2cStop();

    return ret;
}

/**
 * @brief   i2cInit
 * @note    None
 * @param   None
 * @retval  None
 */
int8_t swI2cReadNBytes(uint8_t I2C_Address, uint8_t *Data, uint16_t Datalen)
{
    int8_t ret = 0;
    uint16_t i;

#ifdef I2C_USE_7BIT_ADDR
    I2C_Address <<= 1;
#endif

    swI2cStart();
    swI2cWriteByte((uint8_t)I2C_Address | 0x01);

    if (!swI2cWaitAck())
    {
        swI2cStop();
        ret = -1;
    }
    else
    {
        for (i = 0; i < (Datalen - 1); i++)
        {

            Data[i] = swI2cReadByte(I2C_ACK);
        }
    }

    Data[Datalen - 1] = swI2cReadByte(I2C_N_ACK);

    swI2cStop();

    return ret;
}

#endif