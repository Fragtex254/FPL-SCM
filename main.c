#include <REGX52.H>
#include "LCD1602.h"
#include "MatrixKey.h"
#include "Delay.h"
#include "Buzzer.h"
#include "Key.h"
#include "Servo.h"
#include "AS608.h"
#include "UART.h"

//用到的标志变量
unsigned char Count;																	//键入密码位数
unsigned char InputTime=3;														//输入密码次数
unsigned char KeyNum;																	//矩阵按键编号
unsigned char KNum;																		//独立按键编号
unsigned int Password;																//键入密码值
unsigned int Keyword=1017;														//正确密码
unsigned char Unlocked=0;   													//开锁标志变量

//用到的自定义函数前置声明
void Welcome();																				//欢迎程序
void NumMode();																				//数字键入模式
void ChangePassword();																//更改密码模式
void FingerPrintMode();																//指纹输入模式
void Menu();																					//菜单界面
void Exit();																					//退出声明
//主函数
void main()																						//主函数
{
	Welcome();																					//欢迎程序
	while(1)
	{
		Menu();																						//进入主菜单
	}
}
//欢迎程序
void Welcome()
{
	LCD_Init();
	LCD_ShowString(1,1,"CodeLock Program");
	LCD_ShowString(2,1,"PassWord is:");
	LCD_ShowNum(2,13,Keyword,4);
	while(1)
	{
		KeyNum=MatrixKey();
		if(KeyNum)
		{
			LCD_Init();
			KeyNum=0;
			Buzzer_Time(100);
			return;
		}
	}
}

//菜单程序
void Menu()
{
	LCD_ShowString(1,1,"S13:FPL  S14:NBL");
	LCD_ShowString(2,1,"S15:RES  S16:ESC");
	while(1)
	{
		KeyNum=MatrixKey();
		switch(KeyNum){
		case 13:
			Buzzer_Time(100);																//蜂鸣器响一次，反馈按键被按下
			FingerPrintMode();															//进入指纹模式
			LCD_ShowString(1,1,"S13:FPL  S14:NBL");					//提示信息
			LCD_ShowString(2,1,"S15:RES  S16:ESC");
			break;
		case 14:
			Buzzer_Time(100);																//蜂鸣器响一次，反馈按键被按下
			NumMode();																			//进入键入模式
			LCD_ShowString(1,1,"S13:FPL  S14:NBL");					//提示信息
			LCD_ShowString(2,1,"S15:RES  S16:ESC");
			break;
		case 15:
			Buzzer_Time(100);																//蜂鸣器响一次，反馈按键被按下
			ChangePassword();																//进入密码修改模式
			LCD_ShowString(1,1,"S13:FPL  S14:NBL");					//提示信息
			LCD_ShowString(2,1,"S15:RES  S16:ESC");
			break;
		case 16:
			Buzzer_Time(100);																//蜂鸣器响一次，反馈按键被按下
			Exit();																					//进入退出致谢
			LCD_Init();																			//清屏
			break;
		}
	}
}

//更改密码模式
void ChangePassword()
{
	unsigned int temp=0;																//用于更换密码的过程变量
	LCD_Init();
	LCD_ShowString(1,1,"Password:");										//显示提示信息
	LCD_ShowString(2,5,"T:");														//显示剩余输入次数提示
	LCD_ShowNum(2,7,InputTime,1);												//显示剩余输入次数
	while(1)																						//此处用于检验能否有资格更改密码
	{
		KeyNum=MatrixKey();																//获得键入值
		if(KeyNum)																				//根据键入值分支
		{
			if(KeyNum<=10)																	//如果S1~S10按键按下，输入密码
			{
				Buzzer_Time(100);
				if(Count<4)																		//如果输入次数小于4
				{
					temp*=10;																		//密码左移一位
					temp+=KeyNum%10;														//获取一位密码
					Count++;																		//计次加一	
				}
				LCD_ShowNum(1,13,temp,4);											//更新显示
			}
			if(KeyNum==11)																	//如果S11按键按下，确认
			{
				Buzzer_Time(100);															//蜂鸣器响应，反馈按键被按下
				if(temp==Keyword)															//如果密码等于正确密码
				{
					LCD_ShowString(2,1,"OK ");									//显示OK
					temp=0;																			//密码清零
					Count=0;																		//计次清零
					Unlocked=1;																	//已开锁变量
					InputTime=3;																//重新允许三次错误输入
					LCD_ShowNum(2,7,InputTime,1);
					LCD_ShowNum(1,13,temp,4);										//更新显示
					LCD_ShowString(2,10,"S16:Ent");
				}
				else if(InputTime>0)													//仍有键入次数时
				{
					LCD_ShowString(2,1,"ERR");									//显示ERR
					temp=0;																			//密码清零
					Count=0;																		//计次清零
					InputTime--;
					LCD_ShowNum(1,13,Password,4);								//更新显示
					LCD_ShowNum(2,7,InputTime,1);
				}
				else																					//如果键入次数使用完了
				{
					while(1)																		//进入蜂鸣器报警
					{
						Buzzer_Time(100);
						Delay(100);
					}
				}
			}
			if(KeyNum==12)																	//如果S12按键按下，取消
			{
				Buzzer_Time(100);
				temp=0;																				//密码清零
				Count=0;																			//计次清零
				LCD_ShowNum(1,13,temp,4);												//更新显示
			}
			if(KeyNum==16)  																//如果S16按键按下，退出当前模式
			{
				Buzzer_Time(100);															//按键被按下提示
				if(Unlocked==1)																//如果当时已经开锁则可以跳出循环
				{
					temp=0;				
					Count=0;
					LCD_Init();																	//清屏
					InputTime=3;																//重置键入机会次数
					break;
				}
			else 																						//否则将跳出更改密码的函数体
					return;
			}
		}
	}
	LCD_Init();																					//清屏
	LCD_ShowString(1,1,"New_Password:");								//更改密码指引
	LCD_ShowString(2,10,"S16:Esc");											//退出提示
	while(1)																						//此处开始，用于更换密码
	{
		KeyNum=MatrixKey();																//获得键入的密码值
		if(KeyNum)																				//如果键入了密码
		{
			if(KeyNum<=10)																	//如果S1~S10按键按下，输入密码
			{
				Buzzer_Time(100);															//反馈按键输入
				if(Count<4)																		//如果输入次数小于4
				{
					temp*=10;																		//密码左移一位
					temp+=KeyNum%10;														//获取一位密码
					Count++;																		//计次加一
				}
				LCD_ShowNum(2,1,temp,4);											//更新显示
			}
			if(KeyNum==12)																	//键入12时，删去末位
			{
				Buzzer_Time(100);															//反馈按键输入
				temp/=10;																			//密码去除末位
				Count--;																			//计次减一
				LCD_ShowNum(2,1,temp,4);											//更新显示
			}
			if(KeyNum==11)																	//键入11时，确认更改密码
			{
				Buzzer_Time(100);															//反馈按键输入
				Keyword=temp;																	//更换密码																	
			}
				
			if(KeyNum==16)  																//如果S16按键按下，退出当前模式
			{
				Buzzer_Time(100);															//反馈按键输入
				Unlocked=0;																		//退出重置开锁变量为未开锁状态
				LCD_Init();																		//初始化屏幕
				return;																				//退出
			}
		}
	}
}
	
//指纹识别模式
void FingerPrintMode()
{
	F_Model_Init();																			//初始化+指引
	while(1)
	{
		KeyNum=MatrixKey();																//读取矩阵输入		
		if(KeyNum==13)																		//键入13
		{
			Buzzer_Time(100);																//反馈按键键入
			FPM10A_Find_Fingerprint();											//搜索指纹
			F_Model_Init();																	//重新初始化
		}
		if(KeyNum==14)																		//键入14
		{
			Buzzer_Time(100);																//反馈按键键入
			FPM10A_Add_Fingerprint();												//进入添加指纹模式
			F_Model_Init();																	//重新初始化
		}
		if(KeyNum==15)																		//键入15
		{
			Buzzer_Time(100);																//反馈按键输入
			FPM10A_Delete_All_Fingerprint();								//清空指纹库
			F_Model_Init();																	//重新初始化
		}
		if(KeyNum==16)
		{
			Buzzer_Time(100);																//反馈按键键入
			Lock();																					//上锁
			return;
		}
	}
}


//密码键入模式
void NumMode()
{
	LCD_Init();
	LCD_ShowString(1,1,"Password:");
	LCD_ShowString(2,5,"T:");
	LCD_ShowNum(2,7,InputTime,1);
	LCD_ShowString(2,10,"S16:Esc");
	Count=0;
	while(1)
	{
		KeyNum=MatrixKey();
		if(KeyNum)
		{
			if(KeyNum<=10)																	//如果S1~S10按键按下，输入密码
			{
				Buzzer_Time(100);
				if(Count<4)																		//如果输入次数小于4
				{
					Password*=10;																//密码左移一位
					Password+=KeyNum%10;												//获取一位密码
					Count++;																		//计次加一
				}
				LCD_ShowNum(1,13,Password,4);									//更新显示
			}
			if(KeyNum==11)																	//如果S11按键按下，确认
			{
				Buzzer_Time(100);
				if(Password==Keyword)													//如果密码等于正确密码
				{
					LCD_ShowString(2,1,"OK ");									//显示OK
					Password=0;																	//密码清零
					Count=0;																		//计次清零
					Unlocked=1;																	//已开锁
					InputTime=3;																//重新允许三次错误输入
					LCD_ShowNum(2,7,InputTime,1);
					LCD_ShowNum(1,13,Password,4);								//更新显示
					Unlock();																		//舵机解锁
				}
				else if(InputTime>0)													//否则
				{
					LCD_ShowString(2,1,"ERR");									//显示ERR
					Password=0;																	//密码清零
					Count=0;																		//计次清零
					InputTime--;																//减少输入机会
					LCD_ShowNum(1,13,Password,4);								//更新显示
					LCD_ShowNum(2,7,InputTime,1);
				}
				else																					//机会用完了以后开始报警
				{
					while(1)																		//进入蜂鸣器报警
					{
						Buzzer_Time(100);
						Delay(100);
					}
				}
			}
			if(KeyNum==12)																	//如果S12按键按下，取消
			{
				Buzzer_Time(100);
				Password=0;																		//密码清零
				Count=0;																			//计次清零
				LCD_ShowNum(1,13,Password,4);									//更新显示
			}
			if(KeyNum==16)  																//如果S16案件按下，退出
			{
				Buzzer_Time(100);															//反馈按键按下
				if(Unlocked==1)																//如果当时为解锁的状态
				{
					Lock();																			//解锁
					Unlocked=0;																	//解锁变量置为未解锁
					Password=0;																	//重置密码
				}
				LCD_Init();																		//清屏
				return;
			}
		}
	}
}
//退出程序
void Exit()
{
	LCD_Init();
	LCD_ShowString(1,1,"Thank you !!");
	LCD_ShowString(2,1,"From:");
	LCD_ShowString(2,6,"CJH,WYH,ZWY");								//组员的名字
	while(1)
	{
		
	}
}
