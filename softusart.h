/*
 * @Description:
 * @Author: shexingju && rick.she@suntek-group.com
 * @Date: 2023-08-10 01:50:40
 * @LastEditors: shexingju && rick.she@suntek-group.com
 * @LastEditTime: 2023-08-10 01:51:11
 * @FilePath: softusart.h
 * Copyright 2023 shexingju, All Rights Reserved.
 * 2023-08-10 01:50:40
 */
#ifndef SW_USART_H
#define SW_USART_H

#include "stdint.h"
#include "gd32f30x_it.h"
#include "gd32f30x_exti.h"
#include "systick.h"

#define USART_BUFFER_SIZE 200

// 波特率设置
#define BAUDRATE (9600)

// 分频 =  (prescaler+1)/120000000    period个周期后进入中断
#define EACH_BIT_US (1000000 / BAUDRATE)
#define TIMER_PERIOD (((10 * EACH_BIT_US)) - 1)
#define TIMER_PRESCALER (11)
#define SAMPLING_TIMES (1)
#define USART_BIT_NUMS (10)
#define USART_BIT_SIMPLE (USART_BIT_NUMS * SAMPLING_TIMES)

#define SW_USART_RX_RCU (RCU_GPIOB)
#define SW_USART_RX_PORT (GPIOB)
#define SW_USART_RX_PIN (GPIO_PIN_9)

#define SW_USART_TX_RCU (RCU_GPIOB)
#define SW_USART_TX_PORT (GPIOB)
#define SW_USART_TX_PIN (GPIO_PIN_8)

#define SW_USART_RX_EXTI_PORT (GPIO_PORT_SOURCE_GPIOB)
#define SW_USART_RX_EXTI_PIN (GPIO_PIN_SOURCE_9)

#define SW_USART_EXTI_IRQN (EXTI5_9_IRQn)
#define SW_USART_EXTI (EXTI_9)
#define SW_USART_IRQN_PRIO (2)

#define TX_WRITE_H gpio_bit_set(SW_USART_TX_PORT, SW_USART_TX_PIN)
#define TX_WRITE_L gpio_bit_reset(SW_USART_TX_PORT, SW_USART_TX_PIN)
#define RX_STATUS gpio_input_bit_get(SW_USART_RX_PORT, SW_USART_RX_PIN)

#define DELAY_US(x) delay_1us(x)

typedef enum
{
    RX_IDLE = 0,
    RX_READING,
} RX_STA;

typedef enum
{
    RX_START = 0,
    RX_BITS,
    RX_STOP
} RX_BITS_STA;

typedef struct
{
    uint8_t sw_usart_rxbuf[USART_BUFFER_SIZE];
    uint16_t sw_usart_rx_index;
    uint8_t sw_bit_recv[8];
    uint8_t sw_bit_index;
    RX_STA RX_STATE;
    RX_BITS_STA RX_BIT_STA;
} SW_USART_t;

void sw_usart_send_byte(uint8_t byte);
void sw_usart_send_buf(uint8_t *buffer, uint8_t length);
void sw_usart_init(void);

#endif