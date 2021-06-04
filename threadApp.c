#include <rtthread.h>

/*
 * 程序清单：这是一个串口设备 DMA 接收使用例程
 * 例程导出了 uart_dma_sample 命令到控制终端
 * 命令调用格式：uart_dma_sample uart2
 * 命令解释：命令第二个参数是要使用的串口设备名称，为空则使用默认的串口设备
 * 程序功能：通过串口USART2输出字符串"hello RT-Thread!"，并通过串口USART2接收数据，然后使用USART1打印接收到的数据。
*/


#define SAMPLE_UART_NAME        "uart2"

// 串口接收消息结构体
struct rx_msg
{
  rt_device_t dev;
  rt_size_t size;
};

static rt_device_t serial;
static struct rt_messagequeue rx_mq;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
  struct rx_msg msg;
  rt_err_t result;
  msg.dev = dev;
  msg.size = size;
  // 把串口收到的数据当作一个消息队列发送出去
  result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
  if (result == -RT_EFULL)
  {
    rt_kprintf("message queue full!\n");
  }

  return result;
}

static void serial_thread_entry(void *parameter)
{
  struct rx_msg msg;
  rt_err_t result;
  rt_uint32_t rx_length;

  static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

  while (1)
  {
    rt_memset(&msg, 0, sizeof(msg));
    result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
      // 从串口读取数据
      rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
      rx_buffer[rx_length] = '\0';
      // 将读到的数据再发送出去
      rt_device_write(serial, 0, rx_buffer, rx_length);
      rt_kprintf("%s\n", rx_buffer);
    }
  }
}


static int uart_dma_sample(int argc, char *argv[])
{
  rt_err_t ret = RT_EOK;
  char uart_name[RT_NAME_MAX];
  static char msg_pool[256];
  char str[] = "hello RT-Thread!\r\n";

  if (argc == 2)
  {// 如果命令有2个参数,则第2个参数为设备名
    rt_strncpy(uart_name, argv[1], RT_NAME_MAX);
  }
  else
  {// 否则设备名使用默认名称
    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);
  }

  // 查找系统中的串口设备
  serial = rt_device_find(uart_name);
  if (!serial)
  {
    rt_kprintf("find %s failed!\n", uart_name);
    return RT_ERROR;
  }
  
  // 初始化消息队列
  rt_mq_init(&rx_mq, "rx_mq", msg_pool, sizeof(struct rx_msg), sizeof(msg_pool), RT_IPC_FLAG_FIFO);

  // 以DMA接收 及 轮询发送模式打开串口设备
  rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);

  // 设置接收回调函数
  rt_device_set_rx_indicate(serial, uart_input);

  // 发送字符串
  rt_device_write(serial, 0, str, (sizeof(str) - 1));

  // 创建serial线程
  rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);

  if (thread != RT_NULL)
  {
    rt_thread_startup(thread);
  }
  else
  {
    ret = RT_ERROR;
  }

  return ret;
}

// 测试该命令只能发1次,否则会提示rt_object_init函数出错
MSH_CMD_EXPORT(uart_dma_sample, uart device dma sample);

