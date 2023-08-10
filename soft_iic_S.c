/**
 *******************************************************************************
 * @file        : driver.h
 * @author      : unique
 * @brief       : None
 * @attention   : None
 * @date        : 2023/3/10
 *******************************************************************************
 */

#include <stdint.h>
#include "swI2c.h"
#include "i2cDma.h"
#include <string.h>
#include <stdio.h>

#ifdef USE_SW_I2C
/**
 * @brief   swI2cSclFallingStart
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclFallingStart(swI2cSlaveTypeDef *handler)
{
    handler->SclFallCnt = 0;
    handler->RxIdx = 0;
    handler->TxIdx = 0;
    handler->Flag = 0;
    handler->RxBuf[handler->RxIdx] = 0;
    handler->Rw = I2C_WRITE;
    handler->State = I2C_STA_DATA;
}

/**
 * @brief   swI2cSclFallingData
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclFallingData(swI2cSlaveTypeDef *handler)
{
    handler->SclFallCnt++;

    if (handler->SclFallCnt < 8)
    {
        if (handler->Rw == I2C_READ)
        {
            if (handler->TxBuf[handler->TxIdx] & (1 << (7 - handler->SclFallCnt)))
                SET_SDA_PIN();
            else
                CLR_SDA_PIN();
        }
    }
    else if (8 == handler->SclFallCnt)
    {
        if (handler->Rw == I2C_WRITE)
        {
            if (handler->RxIdx == 0)
            {
                if ((handler->RxBuf[0] & 0xFE) == SW_SLAVE_ADDR)
                {
                    handler->Flag = 1;
                    handler->Rw = handler->RxBuf[0] & 0x01;
                }
            }
            if (handler->Flag)
            {
                SET_SDA_DIR(GPIO_DIR_OUT);
                CLR_SDA_PIN();
            }
        }
        else
        {
            SET_SDA_DIR(GPIO_DIR_IN);
            handler->TxIdx++;
        }

        handler->State = I2C_STA_ACK;
    }
}

/**
 * @brief   swI2cSclRisingAck
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclFallingAck(swI2cSlaveTypeDef *handler)
{
    handler->SclFallCnt = 0;
    if (handler->Rw == I2C_WRITE)
    {
        SET_SDA_DIR(GPIO_DIR_IN);
        handler->RxIdx++;
        handler->RxBuf[handler->RxIdx] = 0;
    }
    else
    {
        SET_SDA_DIR(GPIO_DIR_OUT);
        if (handler->TxBuf[handler->TxIdx] & 0x80)
            SET_SDA_PIN();
        else
            CLR_SDA_PIN();
    }

    handler->State = I2C_STA_DATA;
}

/**
 * @brief   swI2cSclFallingNack
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclFallingNack(swI2cSlaveTypeDef *handler)
{
    handler->SclFallCnt = 0;
    SET_SDA_DIR(GPIO_DIR_IN);
}

/**
 * @brief   swI2cSclRisingData
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclRisingData(swI2cSlaveTypeDef *handler)
{
    if ((I2C_WRITE == handler->Rw) && (handler->SclFallCnt < 8))
    {
        if (handler->sda_state == 1)
        {
            handler->RxBuf[handler->RxIdx] |= (1 << (7 - handler->SclFallCnt));
        }
    }
}

/**
 * @brief   swI2cSclRisingAck
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclRisingAck(swI2cSlaveTypeDef *handler)
{
    if ((handler->Rw == I2C_READ) && (handler->sda_state == 1))
    {
        handler->State = I2C_STA_NACK;
    }
}

/**
 * @brief   swI2cSclFallingProc
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclFallingProc(swI2cSlaveTypeDef *handler)
{
    switch (handler->State)
    {
    case I2C_STA_START:
        swI2cSclFallingStart(handler);
        break;
    case I2C_STA_DATA:
        swI2cSclFallingData(handler);
        break;
    case I2C_STA_ACK:
        swI2cSclFallingAck(handler);
        break;
    case I2C_STA_NACK:
        swI2cSclFallingNack(handler);
        break;
    }
}

/**
 * @brief   swI2cSclRisingProc
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSclRisingProc(swI2cSlaveTypeDef *handler)
{
    switch (handler->State)
    {
    case I2C_STA_DATA:
        swI2cSclRisingData(handler);
        break;
    case I2C_STA_ACK:
        swI2cSclRisingAck(handler);
        break;
    }
}

/**
 * @brief   swI2cSdaFallingProc
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSdaFallingProc(swI2cSlaveTypeDef *handler)
{
    /*  SIGNAL START  */
    if (handler->scl_state == 1)
    {
        handler->State = I2C_STA_START;
    }
}

/**
 * @brief   swI2cSdaRisingProc
 * @note    None
 * @param   None
 * @retval  None
 */
static void swI2cSdaRisingProc(swI2cSlaveTypeDef *handler)
{
    /*  SIGNAL STOP  */
    if (handler->scl_state == 1)
    {
        handler->State = I2C_STA_IDLE;
        if (handler->Rw == I2C_WRITE)
        {
            for (uint8_t i = 1; i < handler->RxIdx; i++)
            {
                i2cDmaWriteByte(handler->RxBuf[i]);
            }
        }
        else if (handler->Rw == I2C_READ)
        {
            handler->set_tx_data_enable = 1;
            handler->TxBuf[0] = 0;
        }
    }
}

/**
 * @brief   swI2cProc
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cProc(swI2cSlaveTypeDef *handler, i2cPinEdgeTypeDef edge)
{
    switch (edge)
    {
    case I2C_SCL_RISING:
        swI2cSclRisingProc(handler);
        break;
    case I2C_SCL_FALLING:
        swI2cSclFallingProc(handler);
        break;
    case I2C_SDA_RISING:
        swI2cSdaRisingProc(handler);
        break;
    case I2C_SDA_FALLING:
        swI2cSdaFallingProc(handler);
        break;
    }
}

/**
 * @brief   swI2cReset
 * @note    None
 * @param   None
 * @retval  None
 */
void swI2cReset(swI2cSlaveTypeDef *handler)
{
    handler->State = I2C_STA_IDLE;
    SET_SDA_DIR(GPIO_DIR_IN);
}

/**
 * @brief   swI2cReset
 * @note    None
 * @param   None
 * @retval  None
 */
int8_t swI2cSetTxData(swI2cSlaveTypeDef *handler, uint8_t *data_p, uint8_t data_size)
{
    if (handler->set_tx_data_enable != 1)
        return -1;

    handler->TxBuf[0] = data_size;
    memcpy(handler->TxBuf + 1, data_p, data_size);

    handler->set_tx_data_enable = 0;

    if (data_size != 0)
        printf("%s->%d, len:%d\n", __FUNCTION__, __LINE__, data_size);

    return 0;
}

#endif
