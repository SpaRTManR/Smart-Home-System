#include "uart.h"

//串口的配置
int  UART_Init(int uart_fd)
{
	//1.定义两个串口结构体
	struct termios  old_attr; //保存串口的旧属性
	struct termios  new_attr; //保存串口的新属性

	//2.保存串口的原本的属性
	tcgetattr (uart_fd,&old_attr);

	//3.配置数据位  8bit
	new_attr.c_cflag &= ~CSIZE; //清空数据位
	new_attr.c_cflag |=  CS8;   //8位数据位

	//4.配置停止位  1bit
	new_attr.c_cflag &= ~CSTOPB; //1位数据位

	//5.配置校验位  none
	new_attr.c_cflag &= ~PARENB; //不使用校验位

	//6.配置波特率  9600bps
	cfsetospeed (&new_attr,B9600);
	cfsetispeed (&new_attr,B9600);

	//7.配置本地连接和接收使能
	new_attr.c_cflag |= CREAD | CLOCAL;

	//8.设置等待时间和最少接收字符
	new_attr.c_cc[VTIME] = 0;
	new_attr.c_cc[VMIN]  = 1;

	//9.设置为原始模式（指的是所有输入的字符都会被当做普通字符）
	cfmakeraw (&new_attr);

	//10.确保参数生效 （立即）
	tcsetattr(uart_fd, TCSANOW,&new_attr);

	//11.清空输入输出缓冲区
	tcflush (uart_fd,TCIOFLUSH);	
}