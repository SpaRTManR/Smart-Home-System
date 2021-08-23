#ifndef	_LCD_H   //可以防止头文件重复包含   #ifndef 和 #endif 是配对使用的
#define _LCD_H


//头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>


//宏定义
#define  LCD_PATH	"/dev/fb0"   //LCD的设备文件
#define  LCD_SIZE	800*480*4    //LCD的大小


//复杂数据类型（结构体、枚举、共用体....）

//保存LCD的设备信息
struct LcdInfo
{
	int fd;  			//保存LCD的文件描述符
	unsigned int * mp;	//保存LCD的内存首地址
};



//函数声明
struct LcdInfo * Lcd_Init(const char *lcd_name);  //对LCD进行初始化


#endif


