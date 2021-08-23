#include "rfid.h"
#include "uart.h"

int uart2_fd;

//串口的配置

//计算校验和
char GetBcc(const char *buf)
{
	char BCC = 0; 
	int i;
	for(i=0; i<(buf[0]-2); i++) 
	{ 
		BCC ^= buf[i]; 
	} 
	return (~BCC);
}

int rfid()
{
	//1.打开串口1  以串口1为例
	uart2_fd = open(UART1_PATH,O_RDWR | O_NOCTTY); //O_NOCTTY 确保现在打开串口不会影响基本串口（终端）
	if(uart2_fd < 0)
	{
		printf("Open UART2 Error\n");
		return -1;
	}

	//2.配置串口1（数据位、停止位、校验位、波特率）
	UART_Init(uart2_fd);
	
	//3.发送请求  数据帧
	char  w_buf[7] = {0}; //保存将要发送的命令
	char  r_buf[8] = {0}; //保存将要收到的信息

	w_buf[0] = 0x07;  			//帧长
	w_buf[1] = 0x02;  			//命令类型
	w_buf[2] = 0x41;  			//命令  请求  'A'
	w_buf[3] = 0x01;  			//数据长度
	w_buf[4] = 0x52;  			//数据信息  0x52  ALL
	w_buf[5] = GetBcc(w_buf);   //校验和  需要计算
	w_buf[6] = 0x03;  			//结束符

	tcflush (uart2_fd,TCIFLUSH);//清空输入缓冲区

	write(uart2_fd,w_buf,sizeof(w_buf)); //发送命令

	usleep(10*1000); 

	read(uart2_fd,r_buf,sizeof(r_buf));  //读取回应

	if(r_buf[2] == 0x00) //状态为0 则表示请求成功
	{
		printf("读卡成功\n");
		//break;
	}
	else{
		printf("读卡失败\n");
	}
	

	char  w_buf_b[8] = {0}; //保存将要发送的命令
	char  r_buf_b[10] = {0}; //保存将要收到的信息
	char  card[4] = {0}; //卡号
	char  str[50],str0[10]; 
	str[0] = '\0';
	int i;
	char card_number[]={"1673012955"};//可通过的卡号

	w_buf_b[0] = 0x08;  			//帧长
	w_buf_b[1] = 0x02;  			//命令类型
	w_buf_b[2] = 0x42;  			//命令  请求  'A'
	w_buf_b[3] = 0x02;  			//数据长度
	w_buf_b[4] = 0x93;  	
	w_buf_b[5] = 0x00; 		//数据信息  0x52  ALL
	w_buf_b[6] = GetBcc(w_buf_b);   //校验和  需要计算
	w_buf_b[7] = 0x03;  			//结束符
	tcflush (uart2_fd,TCIFLUSH);//清空输入缓冲区
	write(uart2_fd,w_buf_b,sizeof(w_buf_b)); //发送命令
	usleep(20*1000); 
	read(uart2_fd,r_buf_b,sizeof(r_buf_b));  //读取回应

	if(r_buf_b[2] == 0x00) //状态为0 则表示请求成功
	{
		printf("获取卡号成功\n");
		for (i = 0; i < 4; i++)
		{
			card[i]=r_buf_b[7-i];
			//printf("card=%d\n",card[i]);
			sprintf(str0,"%d",card[i]);
			strcat(str, str0);
		}
		printf("card_number=%s\n",str);
			// 真实卡号：167 173 92 247
			//247 92 173 167
		int result = strcmp(card_number, str); //比较卡号	
		if (result == 0) 
		{
			printf("ok!!!!!!!!!!!!\n");  
			return 1;
		}
		else printf("wrong!!!!!!!!!!!!\n");			
		//break;	
	}
	else
	{
		printf("失败\n");
		//break;
	}	

	return 0;
}


