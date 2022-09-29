#include <REGX52.H>
#include "LCD1602.h"
#include "MatrixKey.h"
#include "Buzzer.h"
#include "UART.h"
#include "Delay.h"
#include "Servo.h"


unsigned char KeyNum_FP;														//矩阵按键编号
unsigned char ConnectFlag=1;												//未使用到的标志性变量
unsigned char Unlocked_FPM=0;												//指纹模块开锁标志变量


//as608通讯协议定义
volatile unsigned char FPM10A_RECEICE_BUFFER[32];
int finger_id = 0;
//口令验证
code unsigned char FPM10A_Get_Device[10] ={0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};
//协议包头
code unsigned char FPM10A_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  
//获得指纹图像
code unsigned char FPM10A_Get_Img[6] = {0x01,0x00,0x03,0x01,0x00,0x05};    
//获得模版总数
code unsigned char FPM10A_Get_Templete_Count[6] ={0x01,0x00,0x03,0x1D,0x00,0x21 }; 
//搜索指纹搜索范围0 - 999,使用BUFFER1中的特征码搜索
code unsigned char FPM10A_Search_0_9[11]={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x13,0x00,0x21}; 
//搜索0-9号指纹
code unsigned char FPM10A_Search[11]={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x03,0xE7,0x00,0xF8}; 
//将图像放入到BUFFER1
code unsigned char FPM10A_Img_To_Buffer1[7]={0x01,0x00,0x04,0x02,0x01,0x00,0x08}; 
//将图像放入到BUFFER2
code unsigned char FPM10A_Img_To_Buffer2[7]={0x01,0x00,0x04,0x02,0x02,0x00,0x09}; 
//将BUFFER1跟BUFFER2合成特征模版
code unsigned char FPM10A_Reg_Model[6]={0x01,0x00,0x03,0x05,0x00,0x09}; 
//删除指纹模块里所有的模版
code unsigned char FPM10A_Delete_All_Model[6]={0x01,0x00,0x03,0x0D,0x00,0x11};
//将BUFFER1中的特征码存放到指定的位置
volatile unsigned char  FPM10A_Save_Finger[9]={0x01,0x00,0x06,0x06,0x01,0x00,0x0B,0x00,0x19};
//volatile:系统总是重新从它所在的内存读取数据，即使它前面的指令刚刚从该处读取过数据

/*------------------ FINGERPRINT命令字 --------------------------*/
 //发送包头
void FPM10A_Cmd_Send_Pack_Head(void){
	int i;	
	for(i=0;i<6;i++){ 																	//包头 
     Uart_Send_Byte(FPM10A_Pack_Head[i]);   
    }		
}
//发送指令
void FPM10A_Cmd_Check(void){
	int i=0;
	FPM10A_Cmd_Send_Pack_Head(); 												//发送通信协议包头
	for(i=0;i<10;i++){		
		Uart_Send_Byte(FPM10A_Get_Device[i]);
	  }
}
//接收反馈数据缓冲
void FPM10A_Receive_Data(unsigned char ucLength){
  unsigned char i;
  for (i=0;i<ucLength;i++)
     FPM10A_RECEICE_BUFFER[i] = Uart_Receive_Byte();
}
//FINGERPRINT_获得指纹图像命令
void FPM10A_Cmd_Get_Img(void){
    unsigned char i;
    FPM10A_Cmd_Send_Pack_Head(); 											//发送通信协议包头
    for(i=0;i<6;i++){ 																//发送命令 0x1d
		Uart_Send_Byte(FPM10A_Get_Img[i]);
	}
}
//讲图像转换成特征码存放在Buffer1中
void FINGERPRINT_Cmd_Img_To_Buffer1(void){
 	unsigned char i;
	FPM10A_Cmd_Send_Pack_Head(); 												//发送通信协议包头      
   	for(i=0;i<7;i++){   															//发送命令 将图像转换成 特征码 存放在 CHAR_buffer1     
		Uart_Send_Byte(FPM10A_Img_To_Buffer1[i]);
	}
}
//将图像转换成特征码存放在Buffer2中
void FINGERPRINT_Cmd_Img_To_Buffer2(void){
     unsigned char i;
     for(i=0;i<6;i++){    														//发送包头	
		Uart_Send_Byte(FPM10A_Pack_Head[i]);   
   	 }
     for(i=0;i<7;i++){   															//发送命令 将图像转换成 特征码 存放在 CHAR_buffer1     
		Uart_Send_Byte(FPM10A_Img_To_Buffer2[i]);
   	  }
}
//搜索全部用户999枚
void FPM10A_Cmd_Search_Finger(void){
	unsigned char i;	   	    
	FPM10A_Cmd_Send_Pack_Head(); 												//发送通信协议包头
	for(i=0;i<11;i++){         
		Uart_Send_Byte(FPM10A_Search[i]);   
	}
}
void FPM10A_Cmd_Reg_Model(void){
	unsigned char i;	   	    
	FPM10A_Cmd_Send_Pack_Head(); 												//发送通信协议包头
	for(i=0;i<6;i++){          
		Uart_Send_Byte(FPM10A_Reg_Model[i]);   
	}
}
//删除指纹模块里的所有指纹模版
void FINGERPRINT_Cmd_Delete_All_Model(void){
	unsigned char i;    
    for(i=0;i<6;i++){																	//包头
		Uart_Send_Byte(FPM10A_Pack_Head[i]); 
	} 		  
    for(i=0;i<6;i++){ 																//命令合并指纹模版	
		Uart_Send_Byte(FPM10A_Delete_All_Model[i]);   
	}	
}
//保存指纹
void FPM10A_Cmd_Save_Finger( unsigned int storeID ){
	unsigned long temp = 0;
	unsigned char i;
	FPM10A_Save_Finger[5] =(storeID&0xFF00)>>8;
	FPM10A_Save_Finger[6] = (storeID&0x00FF);
	for(i=0;i<7;i++){
		temp = temp + FPM10A_Save_Finger[i];							//计算校验和			 
	}   
	FPM10A_Save_Finger[7]=(temp & 0x00FF00) >> 8; 			//存放校验数据
	FPM10A_Save_Finger[8]= temp & 0x0000FF;		   
	FPM10A_Cmd_Send_Pack_Head(); 												//发送通信协议包头	
	for(i=0;i<9;i++){
		Uart_Send_Byte(FPM10A_Save_Finger[i]);      			//发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
	}
}

//添加指纹
void FPM10A_Add_Fingerprint(){
	LCD_Init();
	LCD_ShowString(1,0,"Finger ID:");
	LCD_ShowString(2,0,"S9:+S10:_S12:OK");
	LCD_ShowNum(1,14,finger_id,3);
	while(1)
	{
		KeyNum_FP=MatrixKey();
			if(KeyNum_FP==9)																//指纹id增加
			{
				Buzzer_Time(100);
				finger_id++;
				finger_id%=300;
				LCD_ShowNum(1,14,finger_id,3);
				}
			if(KeyNum_FP==10)																//指纹id减少
			{
				Buzzer_Time(100);
				finger_id--;
				if(finger_id<0)
				{
					finger_id=299;															//限制0-299
				}
				LCD_ShowNum(1,14,finger_id,3);
			}
			if(KeyNum_FP==12)
			{
				Buzzer_Time(100);
				LCD_Init();
				LCD_ShowString(1,0,"PleasePutFinger1");
				FPM10A_Cmd_Get_Img(); 												//获得指纹图像
				FPM10A_Receive_Data(12);											//判断接收到的确认码,等于0指纹获取成功
				if(FPM10A_RECEICE_BUFFER[9]==0)
				{
					Delay(100);
					FINGERPRINT_Cmd_Img_To_Buffer1();
					FPM10A_Receive_Data(12);
					LCD_Init();
					LCD_ShowString(1,0,"SuccessfulEntry1");
					Buzzer_Time(100);														//蜂鸣器响一次
					Delay(100);
					LCD_ShowString(1,0,"PleasePutFinger2");
					FPM10A_Cmd_Get_Img(); 											//获得指纹图像
					FPM10A_Receive_Data(12);										//判断接收到的确认码,等于0指纹获取成功
				}	 
				if(FPM10A_RECEICE_BUFFER[9]==0)
				{
					Delay(200);
					LCD_Init();
					LCD_ShowString(1,0,"SuccessfulEntry2");
					LCD_ShowString(2,0,"    ID is");
					LCD_ShowNum(2,11,finger_id,3);
					FINGERPRINT_Cmd_Img_To_Buffer2();
					FPM10A_Receive_Data(12);
					FPM10A_Cmd_Reg_Model();											//转换成特征码
					FPM10A_Receive_Data(12); 
					FPM10A_Cmd_Save_Finger(finger_id);                		         
					FPM10A_Receive_Data(12);
					Buzzer_Time(100);														//蜂鸣器响一次
					Delay(1000);
					finger_id+=1;
				}
			}
			if(KeyNum_FP==14)
			{
				LCD_ShowString(1,0,"   Add  finger  ");
				LCD_ShowString(2,0,"    ID is");
				LCD_ShowNum(2,11,finger_id,3);
			}
			if(KeyNum_FP==16){break;}
	}
}


//搜索指纹
void FPM10A_Find_Fingerprint()
{
	unsigned int find_fingerid = 0;
	unsigned char id_show[]={0,0,0};
	LCD_Init();
	LCD_ShowString(1,0," Finger Please ");
	LCD_ShowString(2,0,"S13:OK  S16:Esc");
	while(1)
	{
		KeyNum_FP=MatrixKey();															//获得键值
		switch(KeyNum_FP)
		{
			case 13:
			{
				Buzzer_Time(100);
				FPM10A_Cmd_Get_Img(); 													//获得指纹图像
				FPM10A_Cmd_Get_Img(); 													//获得指纹图像
				FPM10A_Receive_Data(12);												//判断接收到的确认码,等于0指纹获取成功
				if(FPM10A_RECEICE_BUFFER[9]==0)
					{
							Delay(100);
							FINGERPRINT_Cmd_Img_To_Buffer1();
							FPM10A_Receive_Data(12);
							FPM10A_Cmd_Search_Finger();
							FPM10A_Receive_Data(16);
							if(FPM10A_RECEICE_BUFFER[9]==0){ 					//搜索到
								LCD_Init();
								LCD_ShowString(1,0," Search success ");
								LCD_ShowString(2,0,"ID:");
								Buzzer_Time(100);												//蜂鸣器响一次
								find_fingerid = FPM10A_RECEICE_BUFFER[10]*256 + FPM10A_RECEICE_BUFFER[11];
								LCD_ShowNum(2,4,find_fingerid,3);
								LCD_ShowString(2,10,"S16:Esc");
								Delay(1000);
								if(Unlocked_FPM==0)
								{
									Unlock();															//解锁操作
									Unlocked_FPM=1;
								}
							}
							else{
								LCD_Init();
								LCD_ShowString(1,0," Search  failed ");
								LCD_ShowString(2,10,"S16:ESC");
								Buzzer_Time(100);												//蜂鸣器响一次}
							}
					}
					break;
			}
			case 16:
			{
				Unlocked_FPM=0;
				Buzzer_Time(100);
				return;
			}
		}
	}
}


//删除所有存贮的指纹库
void FPM10A_Delete_All_Fingerprint()
{
	LCD_Init();
	LCD_ShowString(1,0,"   Empty All?   ");
	LCD_ShowString(2,0,"S15:YES  S16:NO");
	while(1)
	{
		KeyNum_FP=MatrixKey();															//获取键值
			if(KeyNum_FP==15)
				{
					Delay(1000);
					KeyNum_FP=0;
					KeyNum_FP=MatrixKey();												//获取键值
					if(KeyNum_FP==15)
						{
							LCD_ShowString(1,0,"   emptying     ");							LCD_Init();

							FINGERPRINT_Cmd_Delete_All_Model();
							FPM10A_Receive_Data(12);
							LCD_Init();
							LCD_ShowString(1,0,"   All empty    ");
							Delay(300);
							Buzzer_Time(100);													//蜂鸣器响一次}
					}
				}
			if(KeyNum_FP==16)
			{
				break;
			}
	}
}



//设备检测
void Device_Check(void){
	unsigned char i=0;
	FPM10A_RECEICE_BUFFER[9]=1;														//串口数组第九位可判断是否通信正常
		LCD_ShowString(2,0,"Loading");
		for(i=0;i<8;i++)
		{
			LCD_ShowString(2,i+9,"*");
			Delay(50);
		}
		LCD_ShowString(2,0,"Docking  failure");
		FPM10A_Cmd_Check();																	//单片机向指纹模块发送校对命令
		FPM10A_Receive_Data(12);														//将串口接收到的数据转存
	if(FPM10A_RECEICE_BUFFER[9] == 0)
	{
		LCD_ShowString(2,0,"Docking  success");
		ConnectFlag=0;																			//正确链接的标志变量
	}
}
//指纹模式初始化
void F_Model_Init(){
	{
		LCD_Init();
		Uart_Init();																				//初始化串口
		LCD_ShowString(1,0,"Fingerprint Test");
		Delay(500);																					//延时500MS，等待指纹模块复位
		Device_Check();		   																//校对指纹模块是否接入正确，液晶做出相应的提示
		Delay(1000);																				//对接成功界面停留一定时间
	}
	LCD_ShowString(1,0,"S13:Fin  S14:Add");
	LCD_ShowString(2,0,"S15:Del  S16:ESC");
	
}











