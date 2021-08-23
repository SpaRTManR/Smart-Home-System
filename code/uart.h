#ifndef  __UART_H__
#define  __UART_H__


#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>

#define  UART1_PATH	 "/dev/ttySAC1"  //串口1的路径
#define  UART2_PATH	 "/dev/ttySAC2"  //串口2的路径
#define  UART3_PATH	 "/dev/ttySAC3"  //串口3的路径

#define  AT    				"AT\r\n"				//测试指令
#define  BLE_SET_ANME    	"AT+NAMEGROUP33\r\n"  //设置蓝牙名称


int  UART_Init(int uart_fd);



#endif