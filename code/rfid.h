#ifndef	_RFID_H   //可以防止头文件重复包含   #ifndef 和 #endif 是配对使用的
#define _RFID_H

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

int rfid();


#endif