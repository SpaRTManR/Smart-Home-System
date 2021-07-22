#include "touch.h"


/*******************************************************
*		函数作用	：	对触摸屏进行初始化
*		函数参数	：
*					ts_name  表示触摸屏的文件名
*		返回值	：	ts_fd  表示返回触摸屏的文件描述符
*		注意事项	：  none
*		函数作者	：  xxx
*		函数时间	：  15-07-2021
*		函数版本	：  V1.0
*
* *****************************************************/
int Touch_Init(const char *ts_name)
{
	//1、打开触摸屏  open     只读
	int ts_fd = open(ts_name,O_RDWR);
	if(ts_fd < 0)
	{
		printf("open touch screen error\n");
		return -1;
	}

	return  ts_fd;
}

/*******************************************************
*		函数作用	：	获取一次触摸屏坐标
*		函数参数	：
*					ts_name  表示触摸屏的文件名
*					ts_x  表示保存X轴坐标  变量地址
*					ts_y  表示保存Y轴坐标  变量地址
*		返回值	：	none		
*		注意事项	：  蓝色屏（800,480）黑色屏（1024,600）
*		函数作者	：  xxx
*		函数时间	：  15-07-2021
*		函数版本	：  V1.0
*
* *****************************************************/
void Touch_GetVal(int ts_fd,int *ts_x,int *ts_y)
{
	int cnt = 0; //记录获取的坐标次数

	//2、得到证据  read   定义一个struct input_event类型的变量  & 取地址  sizeof
	struct input_event ts_event;
	while(1)
	{
		read(ts_fd,&ts_event,sizeof(ts_event));

		//3、分析证据  类型 type   if(变量  xxx.type == EV_ABS)  判断是否为触摸屏
		if(ts_event.type == EV_ABS && ts_event.code == ABS_X)
		{
			*ts_x = ts_event.value;
			cnt++;
		}
			
		if(ts_event.type == EV_ABS && ts_event.code == ABS_Y)
		{
			*ts_y = ts_event.value;
			cnt++;
		}	

		if(cnt == 2)
		{
			printf("x=%d,y=%d\n",*ts_x,*ts_y);
			break;
		}
	}
}