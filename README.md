## 工程说明
- USART2的DMA接收及轮询发送例程，USART1用于RT-Thread命令交互；使用STM32F411-atk-nano开发板。
- 串口对应管脚：
    - USART1_TX: PA9
    - USART1_RX: PA10
    - USART2_TX: PA2
    - USART2_RX: PA3
	
> 需要使用**Env**工具的*menuconfig*指令，配置USART2的RX_DMA功能。会在rtconfig.h中增加一个宏订阅BSP_UART1_RX_USING_DMA，
通过rt_hw_board_init() -> rt_hw_usart_init() -> stm32_uart_get_dma_config()配置串口DMA接收功能；