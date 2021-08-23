#include "lcd.h"



/*******************************************************
*		函数作用	：	对LCD屏进行初始化
*		函数参数	：
*					lcd_name  表示LCD的文件名
*		返回值	：	lcd  指的是struct LcdInfo类型的指针
*		注意事项	：  none
*		函数作者	：  xxx
*		函数时间	：  15-07-2021
*		函数版本	：  V1.0
*
* *****************************************************/
struct LcdInfo * Lcd_Init(const char *lcd_name)
{
	//1.定义一个struct LcdInfo的结构体指针
	struct LcdInfo * lcd = malloc(sizeof(struct LcdInfo)); //malloc函数 申请堆空间  返回申请成功的内寸首地址

	//2.打开LCD屏  open     O_RDWR
	lcd->fd = open(lcd_name,O_RDWR);
	if(lcd->fd < 0)
	{
		perror("Open Lcd Error");
	}

	//3.内存映射
	lcd->mp = mmap(	
					NULL,
					LCD_SIZE,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					lcd->fd,
					0);
	if(lcd->mp == MAP_FAILED)
	{
		printf("mmap for lcd error\n");
	}

	return lcd;
}
