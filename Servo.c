#include <REGX52.H>
#include "Delay.h"

static int k=0;
sbit signal=P1^0;						//SG90PMW的引脚接线的时候要注意接到这个口

//旋转度数的宏定义，更改旋转角度
#define spin0 1
#define spin45 2
#define spin90 3
#define spin135 4
#define spin180 5
unsigned int spin;

void timer_init()				//0.5ms定时器初始化
{
	TMOD=0x01;						//定时器0工作模式16位计数器
	ET0=1;								//允许定时器0中断
	TR0=1;								//开启定时器0中断
	EA=1;									//开启总中断
	TH0=0xFE;
	TL0=0x33;							//0.5ms定时对应到Unlock里面那个Delay(500)
	
}

void timer_close()
{
	ET0=0;								//不允许定时器0中断
	TR0=0;								//关闭定时器0中断
	EA=0;									//关闭总中断
}

void Timer0() interrupt 1
{
	TR0=0;								//关闭计数
	k=k+1;
	TH0=0xFE;
	TL0=0x33;							//重置使得下次定时器只计时0.5ms
	if(k<=spin)
	{
		signal=1;
	}
	else
	{
		signal=0;
		if(k==40)
		{
			k=0;
		}
	}
	TR0=1;								//开启计数
	
}
void delayms(int ms)
{
	int i,j;
	for(j=ms;j>0;j--)
		for(i=110;i>0;i--);
}
void Unlock()
{
		P1=0x00;
		timer_init();				//0.5ms定时器初始化
		spin=spin180;
		delayms(500);
		timer_close();			//关闭其中断
}
void Lock()
{
	P1=0x00;
	timer_init();					//0.5ms定时器初始化
	spin=spin0;
	delayms(500);
	P1=0x00;
	timer_close();				//关闭其中断

}

