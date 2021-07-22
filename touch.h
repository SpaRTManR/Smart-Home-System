#ifndef	_TOUCH_H
#define _TOUCH_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>  //触摸屏


#define  TS_PATH	"/dev/input/event0"  //触摸屏的文件路径



int  Touch_Init(const char *ts_name); //对触摸屏进行初始化

void Touch_GetVal(int ts_fd,int *ts_x,int *ts_y);  //获取一次触摸屏的坐标

#endif