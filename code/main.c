/****************************************************************************
*	C语言中规定  头文件的包含形式有两种 ""  <>
*
*   #include <xxx.h>  编译器只从系统标准路径（/usr/include）中查找头文件
*
*	#include "xxx.h"  编译器先从当前路径中查找，没找到再去系统路径中找，最后在报错
*
* **************************************************************************/



#include "lcd.h"
#include "touch.h"
#include "bmp.h"
#include "camera.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "uart.h"
#include "rfid.h"
#define PIC_NUM 3

extern volatile int video_flag;
extern volatile int photo_flag;
extern volatile int camera_flag;
extern volatile  int capture_num;
extern volatile int video_num;
extern volatile int state; //标识录视频的个数



int ts_x,ts_y; //保存触摸屏坐标
int ts_fd;
int uart2_fd;
char buf[50] = {0}; //

void * ts_funtion(void * arg)
{
	while(1)
	{
		Touch_GetVal(ts_fd,&ts_x,&ts_y); //不断获取坐标
	}
}

void * camera_funtion(void * arg)
{
	while(1)
	{
		get_camera_picture(); //不断显示摄像头画面
	}
}

void * ble_function(void * arg)
{
	char recv_buf[50] = {0};
	while(1)
	{
		read(uart2_fd,recv_buf,sizeof(recv_buf));
		printf("recv data from ble is %s\n", recv_buf);
		strcpy(buf,recv_buf);
		

		bzero(recv_buf,sizeof(recv_buf));
		tcflush (uart2_fd,TCIOFLUSH);
	}
}




int main(int argc, char const *argv[])
{
	/*****************************
	*
	*       定义初始变量       
	*
	******************************/
	int flag; //记载在哪个页面
	int i; //用于循环计数
	int pwd_corret[6] = {1,2,3,4,5,6}; // 正确密码
	int pwd[6];


	/*****************************
	*
	*    打开LCD
	*    打开触摸屏
	*    开启两个线程
	*
	******************************/

	//1.打开LCD
	struct LcdInfo *lcd =  Lcd_Init(LCD_PATH);

	//2.打开触摸屏
	ts_fd = Touch_Init(TS_PATH);
	
	pthread_t  ts_thread,camera_thread;


	pthread_create(&ts_thread,NULL,ts_funtion,NULL); //为触摸屏创建的线程
	pthread_create(&camera_thread,NULL,camera_funtion,NULL);//摄像头线程


	/*******************************
	*
	*
	*     开启蓝牙通信
	*     开启蓝牙线程
	*
	*
	*******************************/

	
	//1.打开串口1  以串口1为例
	uart2_fd = open(UART2_PATH,O_RDWR | O_NOCTTY); //O_NOCTTY 确保现在打开串口不会影响基本串口（终端）
	if(uart2_fd < 0)
	{
		printf("Open UART2 Error\n");
		return -1;
	}

	//2.配置串口1（数据位、停止位、校验位、波特率）
	UART_Init(uart2_fd);


	//3.创建一条线程（蓝牙接收数据）
	pthread_t  ble_thread;

	pthread_create(&ble_thread,NULL,ble_function,NULL); //接收蓝牙收到的数据

	//4.发送对应的AT指令
	write(uart2_fd,AT,strlen(AT));
	usleep(50*1000);

	write(uart2_fd,BLE_SET_ANME,strlen(BLE_SET_ANME));
	usleep(50*1000);

	
	
	/****************************************************************************
*	flag：
*   # 0 --- 主页面
*   # 1 --- 选择页面
*	# 3 --- 密码登录
*   
*   # 5 --- 刷卡页面
*   # 9 --- 菜单页面
*   # 11 --- 图片显示
*   # 12 --- 监控页面
*   # 13 --- 视频页面
*
* **************************************************************************/


	
	//3.显示登陆界面
	//Bmp_Show(lcd,"./xxx.bmp");

	char gif_name[50] = {0};
	char pic_name[50] = {0};	
	
	for(i=0;i<63;i++)
	{
		sprintf(gif_name,"./pic/Frame%d.jpg",i); //生成图片名字
		show_jpg(lcd,0,0,gif_name);
		usleep(5*1000);//  20ms 标准C库中提供了2个延时函数  sleep()  秒   usleep()  微秒   1ms = 1000us
	}
	
	

	//4.获取坐标
	while(1)
	{

		if(flag == 0) Bmp_Show(lcd,"./img/login.bmp");

	    
		//Touch_GetVal(ts_fd,&ts_x,&ts_y);

		if(flag == 9){
			flag = 9;
			goto flag9;
		}

		if((ts_x > 200 && ts_x < 400 && ts_y > 290 && ts_y < 410 && flag == 0) || (flag == 1) || strcmp(buf,"1") == 0){
			bzero(buf,sizeof(buf));
			printf("Already in the system!");
			flag = 1; //页面在选择页面
			while(1){
				//若点击密码登录
				Bmp_Show(lcd,"./img/choose.bmp");
				//Touch_GetVal(ts_fd,&ts_x,&ts_y);
				
				
			
				if(ts_x > 275 && ts_x < 670 && ts_y >105 && ts_y < 280  || strcmp(buf,"3") == 0){
					bzero(buf,sizeof(buf));
					flag = 3; //标志进入密码选择页
					ts_x = 0;
				    ts_y = 0;
					break;
					//printf("flag = %d\n",flag ); 调参用的
				}
				
				//若点击刷卡登录
				if(ts_x > 275 && ts_x < 670 && ts_y > 320 && ts_y < 450 || strcmp(buf,"5") == 0){
					//flag = 9; //标志进入刷卡选择页面
					bzero(buf,sizeof(buf));
					flag = 5;
					ts_x = 0;
				    ts_y = 0;
					break;
					//printf("flag = %d\n",flag );  调参用的
				}

			}

		

			if(flag == 3){
				for(i = 0; i < 6 ; i ++) pwd[i] = 0;
				int cnt = 0;//用于记录当前输入密码位数
				int wrong_cnt = 0; //出错次数
				while(1){
					switch(cnt){
						case 0 :Bmp_Show(lcd,"./img/pwd0.bmp");break;
						case 1 :Bmp_Show(lcd,"./img/pwd1.bmp");break;
						case 2 :Bmp_Show(lcd,"./img/pwd2.bmp");break;
						case 3 :Bmp_Show(lcd,"./img/pwd3.bmp");break;
						case 4 :Bmp_Show(lcd,"./img/pwd4.bmp");break;
						case 5 :Bmp_Show(lcd,"./img/pwd5.bmp");break;
					}

					

					if(strcmp(buf,"123456") == 0){
						flag = 9;
						break;
					}
				
				
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);

						
					if(ts_x > 360 && ts_x < 410  && ts_y > 250 && ts_y < 310 ){
						pwd[cnt ++] = 1;
						printf("num = 1\n");
						ts_x = 0;
						ts_y = 0;
					} 
						

					if(ts_x > 490 && ts_x < 533 && ts_y > 250 && ts_y < 310){
						 pwd[cnt ++] = 2;
						 printf("num = 2\n");
						 ts_x = 0;
					     ts_y = 0;
					}
	
								
					if(ts_x > 605 && ts_x < 650 && ts_y > 250 && ts_y < 310){
						pwd[cnt ++] = 3;
						printf("num = 3\n");
						ts_x = 0;
						ts_y = 0;
					} 
	
								
					if(ts_x > 360 && ts_x < 410 && ts_y > 340 && ts_y < 395){
						pwd[cnt ++] = 4;
						printf("num = 4\n");
						ts_x = 0;
						ts_y = 0;
					} 

					if(ts_x > 490 && ts_x < 533 && ts_y > 340 && ts_y < 396){
						pwd[cnt ++] = 5;
						printf("num = 5\n");
						ts_x = 0;
						ts_y = 0;
					} 

					if(ts_x > 605 && ts_x < 650 && ts_y > 340&& ts_y <  395){
						pwd[cnt ++] = 6;
						printf("num = 6\n");
						ts_x = 0;
						ts_y = 0;
					} 
					if(ts_x > 360 && ts_x < 410 && ts_y > 435 && ts_y < 480){
						pwd[cnt ++] = 7;
						printf("num = 7\n");
						ts_x = 0;
						ts_y = 0;
					} 
					if(ts_x > 490 && ts_x < 533 && ts_y > 435 && ts_y < 480){
						pwd[cnt ++] = 8;
						printf("num = 8\n");
						ts_x = 0;
						ts_y = 0;
					} 
					if(ts_x > 605 && ts_x < 650 && ts_y > 435 && ts_y < 480 ){
						pwd[cnt ++] = 9;
						printf("num = 9\n");
						ts_x = 0;
						ts_y = 0;
					} 
					if(ts_x > 490 && ts_x < 533 && ts_y > 513 && ts_y < 595 ){
						pwd[cnt ++] = 0;
						printf("num = 0\n");
						ts_x = 0;
						ts_y = 0;
					} 
					if(ts_x > 605 && ts_x < 650 && ts_y > 513 && ts_y < 595 ){
						cnt --;
						printf("num = -1\n");
						ts_x = 0;
						ts_y = 0;
					} 

					if(cnt < 0){
						flag = 1;
                    	break;
					}  //直接按返回则退出

					int test_pwd = 1;

					if(cnt >= 6){
						if(pwd[0]!=pwd_corret[0]) test_pwd = 0;
						if(pwd[1]!=pwd_corret[1]) test_pwd = 0;
						if(pwd[2]!=pwd_corret[2]) test_pwd = 0;
						if(pwd[3]!=pwd_corret[3]) test_pwd = 0;
						if(pwd[4]!=pwd_corret[4]) test_pwd = 0;
						if(pwd[5]!=pwd_corret[5]) test_pwd = 0;
						
						if(test_pwd){
							flag = 9;
							printf("%d\n",test_pwd );
							break;
						}else{
							cnt = 0;
						    wrong_cnt ++;
						    printf("%d\n",wrong_cnt );
						}

						if(wrong_cnt >= 3){
							flag = 1;
							wrong_cnt = 0;
							break;
							
						}

					}


				    
						

				}
						
			}

			flag9:printf("here\n");


			//菜单页面
			if(flag == 9){
				while(1){
					Bmp_Show(lcd,"./img/info.bmp");
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);
					if(ts_x > 795 && ts_x < 1010 &&  ts_y > 315 && ts_y < 500  || strcmp(buf,"pic") == 0){
						bzero(buf,sizeof(buf));
						ts_x = 0;
						ts_y = 0;
						flag = 11; //进入相册页面
						break;
					}
				    if(ts_x > 795 && ts_x < 1010 &&  ts_y > 75 && ts_y < 210  || strcmp(buf,"cap") == 0){
						bzero(buf,sizeof(buf));
						ts_x = 0;
						ts_y = 0;
						flag = 12;//监控页面
						break;
					}
					if(ts_x > 85 && ts_x < 235 && ts_y > 325 && ts_y < 480 || strcmp(buf,"vid") == 0){
						bzero(buf,sizeof(buf));
						ts_x = 0;
						ts_y = 0;
						flag = 13;//进入视频页面
						break;
					} 
					
					if(ts_x > 4 && ts_x < 54 &&  ts_y > 7 && ts_y < 38  || strcmp(buf,"out") == 0){
						bzero(buf,sizeof(buf));
						printf("login out\n");
						ts_x = 0;
						ts_y = 0;
						flag = 0; // 退出
						break;
					}

					
				}
			}

			//相册页面
			if(flag == 11){

				int j = 1;
				int bmp_cnt = 0;
				int jpg_cnt = 0;
				char bmp_name[50] = {0};
				char jpg_name[50] = {0};
				while(1){
					sprintf(bmp_name,"./img/pic_%d.bmp",bmp_cnt);
					if(access(bmp_name, F_OK) == 0) bmp_cnt ++;
					else break;
				}
				printf("bmp_cnt = %d\n",bmp_cnt);
				while(1){
					sprintf(jpg_name,"./img/capture_%d.jpg",jpg_cnt);
					if(access(jpg_name,F_OK) == 0) jpg_cnt ++;
					else break;
				}
				printf("jpg_cnt = %d\n",jpg_cnt);
				int all_cnt = bmp_cnt + jpg_cnt;
				
				while(1){
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);
					
					if(ts_x > 0 && ts_x < 100 && ts_y > 0 && ts_y < 100  || strcmp(buf,"H") == 0) //左上角
					{
						//left:printf("left");
						bzero(buf,sizeof(buf));
						if(j > 1 ) j --;
						else j = j + all_cnt  - 1; 
						ts_x = 0;
						ts_y = 0;
					}
					if(ts_x > 900 && ts_x < 1024 && ts_y > 0 && ts_y < 100 || strcmp(buf,"J") == 0) //右上角
					{
						//right:printf("right");
						bzero(buf,sizeof(buf));
						if(j <  all_cnt ) j ++;
						else j  = j  - all_cnt + 1 ;
						ts_x = 0;
						ts_y = 0;
					}
					if(ts_x > 350 && ts_x < 650 && ts_y > 450 && ts_y < 600 || strcmp(buf,"out") == 0 ) //中间下部
					{
						//out:printf("out");
						bzero(buf,sizeof(buf));
						j = 100;
						ts_x = 0;
						ts_y = 0;
					}

					printf("j = %d\n",j );

				
					if(j == 100){
						flag = 9;
						//printf("111111111111\n");
						break;
					}
					if(j  <= bmp_cnt) {
						sprintf(pic_name,"./img/pic_%d.bmp",j - 1);
						Bmp_Show(lcd,pic_name);
					}else{
						Bmp_Show(lcd,"./img/camera.bmp");
						sprintf(pic_name,"./img/capture_%d.jpg",j - bmp_cnt - 1);
						show_jpg(lcd,240,60,pic_name);

					
					}
					
				}
				
					
				//printf("test\n"); debug
				printf("flag = %d\n",flag ); 
				printf("x = %d, y = %d\n",ts_x,ts_y );
			}

			//printf("test02\n"); //debug


			if(flag == 12){
		
				Bmp_Show(lcd,"./img/camera.bmp");
				while (1){
					camera_flag = 1;
					// if(strcmp(buf,"capvid") == 0) goto capvid;
					// if(strcmp(buf,"cappic") == 0) goto cappic;
					// if(strcmp(buf,"out") == 0) goto loginout;
					
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);
					if(ts_x > 0 && ts_x < 100 && ts_y > 0 && ts_y < 100  || strcmp(buf,"out") == 0){
						//loginout:printf("loginout");
						bzero(buf,sizeof(buf));
						camera_flag = 0;
						flag = 9;
						ts_x = 0;
						ts_y = 0;
						break;
					}

					if(ts_x > 25 && ts_x < 175 && ts_y > 150 && ts_y < 320  || strcmp(buf,"cappic") == 0){

						cappic:
						photo_flag = 1;
						ts_x = 0;
						ts_y = 0;
						bzero(buf,sizeof(buf));
						printf("camera_flag = 2");
					
					}

					if(ts_x > 460 && ts_x < 605 && ts_y > 395 && ts_y < 480  || strcmp(buf,"capvid") == 0){

				
						video_flag = ~video_flag;
						if(video_flag) state ++;
						ts_x = 0;
						ts_y = 0;
						bzero(buf,sizeof(buf));
						printf("start to vedio");
						printf("video_flag = %d",video_flag);
					}

				}
			}

			//视频页面
			if(flag == 13){
				int v_cnt = 1; //计数照片数量
				int v = 0;
				int p = 0;
				char video_name[150] = {0};
				Bmp_Show(lcd,"./img/camera.bmp");
				sprintf(video_name,"./video/v0photo0.jpg"); //初始图片
				while(1){
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);
					if(ts_x > 845 && ts_x < 1022 && ts_y > 110 && ts_y < 325 || strcmp(buf,"J") == 0)
					{
						ts_x = 0 ;
						ts_y = 0 ;
						bzero(buf,sizeof(buf));
						v++;//下一个视频
						printf("v=%d\n",v );
						if(v>2) v=0;
						sprintf(video_name,"./video/v%dphoto0.jpg",v); //生成图片名字
						show_jpg(lcd,240,60,video_name);
					}
					if(ts_x > 25 && ts_x < 175 && ts_y > 110 && ts_y < 320 || strcmp(buf,"H") == 0)
					{
						ts_x = 0  ;
						ts_y = 0 ;
						bzero(buf,sizeof(buf));
						v--;//上一个视频
						printf("v=%d\n",v );
						if(v<0) v=2;
						sprintf(video_name,"./video/v%dphoto0.jpg",v); //生成图片名字
						show_jpg(lcd,240,60,video_name);
					}
					if(ts_x > 775 && ts_x < 905 && ts_y > 450 && ts_y < 540 || strcmp(buf,"out") == 0){
						ts_x = 0;
						ts_y = 0;
						bzero(buf,sizeof(buf));
						flag = 9;
						goto flag9;
						printf("flag = %d",flag);
					}
					//Touch_GetVal(ts_fd,&ts_x,&ts_y);
					while(ts_x > 240 && ts_x < 560 && ts_y > 60 && ts_y < 300 || strcmp(buf,"action") == 0){
						ts_x = 0;
						ts_y = 0;
						bzero(buf,sizeof(buf));

						Bmp_Show(lcd,"./img/camera.bmp");
						show_jpg(lcd,240,60,video_name);
						while(v_cnt --){
	
						sprintf(video_name,"./video/v%dphoto%d.jpg",v,p++);
						printf("p = %d",p);
						if(access(video_name,F_OK) == 0){
							v_cnt ++;
						}else{
							break;
						}
						printf("v_cnt = %d",v_cnt);
						show_jpg(lcd,240,60,video_name);
						usleep(10000);
						printf("ts_x = %d",ts_x); 
						// while(ts_x > 25 && ts_x < 175){
						// 	printf("stop!");
						// }
						}
						p = 0;
						v = 0;
						v_cnt = 1;
						sprintf(video_name,"./video/v%dphoto0.jpg",v); //初始图片
						ts_x = 0;
						ts_y = 0;
						break;



					}
					
				}
				
				
				
				
				}

				
			

			if(flag == 5){
				while(1){
					Bmp_Show(lcd,"./img/NFC.bmp");
				    //Touch_GetVal(ts_fd,&ts_x,&ts_y);
					if(rfid() == 1){
						printf("刷卡进入成功");
						flag = 9;
						break;
					}
					if(ts_x > 0 && ts_x < 160  || strcmp(buf,"out") == 0) //左上角
					{
						bzero(buf,sizeof(buf));
						flag = 1;
						ts_x = 0;
						ts_y = 0;
						break;
					}

				}
				

			}


		}


		
		// if(ts_x > 0 && ts_x < 100 && ts_y > 0 && ts_y < 100) //左上角
		// {
		// 	printf("hello\n");
		// }
		// if(ts_x > 900 && ts_x < 1014 && ts_y > 0 && ts_y < 100) //右上角
		// {
		// 	printf("world\n");
		// }
	}

	return 0;
}