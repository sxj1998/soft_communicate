/*
 * @Author: shexingju SUNTEK
 * @Date: 2023-07-14 09:18:41
 * @LastEditTime: 2023-08-10 01:50:48
 * @LastEditors: shexingju && rick.she@suntek-group.com
 * @Description:
 * @FilePath: softusart.c
 * 版权声明
 */
#include "sw_usart.h"
#include "log.h"
#include "common.h"
#include "string.h"

SW_USART_t SW_USART_OBJ;
uint8_t timerFlag = 0;

/**
 * @description: gpio 初始化
 * @return {*}
 * @use:
 */
void sw_usart_gpio_init(void)
{
    rcu_periph_clock_enable(SW_USART_RX_RCU);
    gpio_init(SW_USART_RX_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SW_USART_RX_PIN);

    rcu_periph_clock_enable(SW_USART_TX_RCU);
    gpio_init(SW_USART_TX_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SW_USART_TX_PIN);
    TX_WRITE_H;
}

/**
 * @description: RX 脚外部中断初始化
 * @return {*}
 * @use:
 */
void rx_exti_init(void)
{
    rcu_periph_clock_enable(RCU_AF);
    nvic_irq_enable(SW_USART_EXTI_IRQN, SW_USART_IRQN_PRIO, 0);
    gpio_exti_source_select(SW_USART_RX_EXTI_PORT, SW_USART_RX_EXTI_PIN);
    exti_init(SW_USART_EXTI, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(SW_USART_EXTI);
}

/**
 * @description: 定时器初始化
 * @return {*}
 * @use:
 */
void rx_timer_init(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER5); /* 使能TIMER5时钟 */
    /* TIMER5 初始化配置 */
    timer_deinit(TIMER5);
    timer_initpara.prescaler = TIMER_PRESCALER;         /* 时钟预分频系数 */
    timer_initpara.period = TIMER_PERIOD;               /* 自动重装载值 */
    timer_initpara.counterdirection = TIMER_COUNTER_UP; // 向上计数模式

    timer_init(TIMER5, &timer_initpara);
    timer_enable(TIMER5);
    nvic_irq_enable(TIMER5_IRQn, 3, 0);
    //  timer_interrupt_enable(TIMER5,TIMER_INT_UP);    /* 使能更新中断 */
    timer_interrupt_disable(TIMER5, TIMER_INT_UP);
}

/**
 * @description: 开启定时器中断，设置装载值
 * @return {*}
 * @use:
 */
void start_timer(void)
{
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);
    timer_counter_value_config(TIMER5, TIMER_PERIOD);
}

/**
 * @description: 关闭定时器中断
 * @return {*}
 * @use:
 */
void stop_timer(void)
{
    timer_interrupt_disable(TIMER5, TIMER_INT_UP);
}

/**
 * @description: 参数初始化
 * @return {*}
 * @use:
 */
void sw_usart_parm_init(void)
{
    SW_USART_OBJ.RX_STATE = RX_IDLE;
    SW_USART_OBJ.RX_BIT_STA = RX_STOP;
    SW_USART_OBJ.sw_usart_rx_index = 0;
    memset(SW_USART_OBJ.sw_bit_recv, 0, 8);
    SW_USART_OBJ.sw_bit_index = 0;
}

/**
 * @description: 外设deinit
 * @return {*}
 * @use:
 */
void sw_usart_deinit(void)
{
    nvic_irq_disable(SW_USART_EXTI_IRQN);
    exti_deinit();
    gpio_deinit(SW_USART_RX_PORT);
}

/**
 * @description: 外设从从初始化
 * @return {*}
 * @use:
 */
void sw_usart_init(void)
{
    sw_usart_gpio_init();
    rx_exti_init();
    rx_timer_init();
    sw_usart_parm_init();
}

/**
 * @description: 发送单字节串口
 * @return {*}
 * @use:
 */
void sw_usart_send_byte(uint8_t byte)
{
    TX_WRITE_L;
    DELAY_US(EACH_BIT_US);
    for (int i = 0; i < 8; i++)
    {
        if (byte & 0x01)
            TX_WRITE_H;
        else
            TX_WRITE_L;

        DELAY_US(EACH_BIT_US);
        byte = byte >> 1;
    }
    TX_WRITE_H;
    DELAY_US(EACH_BIT_US);
}

/**
 * @description: 发送多字节串口数据
 * @return {*}
 * @use:
 */
void sw_usart_send_buf(uint8_t *buffer, uint8_t length)
{
    for (int i = 0; i < length; i++)
        sw_usart_send_byte(buffer[i]);
}

/**
 * @description: 接收串口数据
 * @return -1：start信号错误 1：单字节接收完成 0：计数接收1bit数据
 * @use:
 */
int8_t sw_usart_data_recv(uint8_t *recvedData)
{
    switch (SW_USART_OBJ.RX_BIT_STA)
    {
    case RX_START:
        if (RX_STATUS == 0)
        {
            SW_USART_OBJ.RX_BIT_STA = RX_BITS;
            SW_USART_OBJ.RX_STATE = RX_READING;
            SW_USART_OBJ.sw_bit_index = 0;
            memset(SW_USART_OBJ.sw_bit_recv, 0, 8);
        }
        else
            return -1;
        break;

    case RX_BITS:
        SW_USART_OBJ.sw_bit_recv[SW_USART_OBJ.sw_bit_index] = RX_STATUS;
        SW_USART_OBJ.sw_bit_index++;
        *recvedData |= SW_USART_OBJ.sw_bit_recv[SW_USART_OBJ.sw_bit_index - 1] << (SW_USART_OBJ.sw_bit_index - 1);
        if (SW_USART_OBJ.sw_bit_index % 8 == 0)
            SW_USART_OBJ.RX_BIT_STA = RX_STOP;
        break;

    case RX_STOP:
        if (RX_STATUS == 1)
        {
            stop_timer();
            return 1;
        }
        break;
    default:

        break;
    }
    return 0;
}

void EXTI5_9_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(SW_USART_EXTI))
    {
        if (SW_USART_OBJ.RX_BIT_STA == RX_STOP)
        {
            SW_USART_OBJ.RX_BIT_STA = RX_START;
            start_timer();
        }
    }
    exti_interrupt_flag_clear(SW_USART_EXTI);
}

void TIMER5_IRQHandler(void)
{
    int8_t ret = 0;
    timer_flag_clear(TIMER5, TIMER_FLAG_UP); /* 定时器更新中断的标识位需要手动清除 */

    ret = sw_usart_data_recv(&SW_USART_OBJ.sw_usart_rxbuf[SW_USART_OBJ.sw_usart_rx_index]);
    if (ret == 1)
    {
        SW_USART_OBJ.sw_usart_rx_index++;
        SW_USART_OBJ.RX_STATE = RX_IDLE;
    }
    else if (ret == -1)
    {
        SW_USART_OBJ.RX_STATE = RX_IDLE;
    }
    else
    {
        ;
    }

    if (SW_USART_OBJ.sw_usart_rx_index >= USART_BUFFER_SIZE)
    {
        SW_USART_OBJ.sw_usart_rx_index = 0;
        memset(SW_USART_OBJ.sw_usart_rxbuf, 0, USART_BUFFER_SIZE);
    }
}
