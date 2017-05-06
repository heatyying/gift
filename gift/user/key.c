 bit flag=0; 定义标志
以下是键盘处理程序：
void scankey()   //扫描按键
{
	if(p10==0&&flag==0)  //p10口按键及处理程序，其中flag如果为1就不用再扫描    就是这里，没有消抖动了~是怎么回事咧
	{
		delay(10);
		flag=1;
		delay(5);
	}
	if(p10==1&&flag==1)  //如果p10为1，flag为1说明已经被按下并抬起
	{
		TR0=!TR0;    //松开时相应
		flag=0;    //标志清零，为下次按键做准备（其实为一次处理）
	}
}


以下是另一个一个检测的主要部分：
#include <reg51.h>
#include <intrins.h>
#define key_port P0    //键盘接口定义
sbit key_port_0=key_port^0;
sbit key_port_1=key_port^1;
sbit key_port_2=key_port^2;
sbit key_port_3=key_port^3;

if(Key_down==PRESSED)
{
     while(Key_down==PRESSED)          //计数，用来判断是长按还是短按
     {
  
      if( (counter++) > Long_cnt) //counter>65534:长按，
        {
     Key_Code=KEY_FP_DOWN;             //KEY1长按时返回的键值
    // counter=0;
           return;
        }
     }
     if(counter<=Long_cnt && counter>Short_cnt)   //counter<=65534:短按，counter>20:消抖动
     {    
     
      Key_Code=KEY_FP_DOWN;             //KEY1短按时返回的键值
   // counter=0;
           return;        
     } 
}
 
以下程序可以作为一个单独的 key.c 文件包含在项目里面：
#define KEYDELAY 25    /*首键延迟次数*/
#define KEYLOOP 10    /*连续键间隔次数,需小于KEYDELAY*/
unsigned char key_bak;
unsigned char key_temp;
unsigned char key_old;
unsigned char key_data;
bit key_ok;
    //  P1口行列式键盘  //
#define KEYPIN_L  P1               // 定义键扫描列端口为 P1低四位输入  //
#define KEYPIN_H  P1              // 定义键扫描行端口为 P1高四位扫描输出  //
    //内部私有函数  //
unsigned char fnKeycode(unsigned char key);     //  根据键盘映射表输出顺序键值            //
    //  P1口行列式键盘  //
//---------------------------------------------------------------------------//
void KeyScan(void)                    //键扫描
{
    unsigned char sccode,recode;
    KEYPIN_L = KEYPIN_L|0x0f;                    //  P1低四位为列线输入         //
    KEYPIN_H = KEYPIN_H&0x0f;                    //  P1高四位为行线发全零扫描码 //
    if ((KEYPIN_L&0x0f) !=  0x0f)    //有键按下
        {
        key_temp = 键值;             //键扫描，获得键值
        if (key_temp == key_bak)
            {
            key_old++;
          //  if (key_old==KEYDELAY)                //连续键,不要则把此if屏蔽
             //   {
               // key_ok=1;
              //  key_data=fnKeycode(key_temp);    //键*
               // key_old=KEYDELAY-KEYLOOP;
          //  }
        }
        else
            {
            key_old=0;
            key_bak=key_temp;
        }
    }
    else if (key_old)                       //键抬起
    {
		key_ok=1;
		key_data=fnKeycode(key_temp);    //键*
		if (key_old==100)                //加上时间,不同时间不同功能,与连续键冲突
		{
			bp(1);
			key_data    +=    0x10;
		}
		if (key_old==200)
		{
			bp(2);
			key_data    +=    0x20;
		}

	}
	key_old=0;
}
//---------------------------------------------------------------------------//
unsigned char KeyGetcode(void)        //返回按键码
{
    key_ok=0;
    return(key_data);
}
bit KeyTest(void)                //检查有无按键
{
    KeyScan();            //键扫描
    return(key_ok);
}
bit flag=0; 定义标志
	
以下是键盘处理程序：
void scankey()   //扫描按键
{
	if(p10= =0&&flag= =0)  //p10口按键及处理程序，其中flag如果为1就不用再扫描    就是这里，没有消抖动了~是怎么回事咧
	{
		delay(10);
		flag=1;
		delay(5);
	}
	if(p10= =1&&flag= =1)  //如果p10为1，flag为1说明已经被按下并抬起
	{
		TR0=!TR0;    //松开时相应
		flag=0;    //标志清零，为下次按键做准备（其实为一次处理）
	}
}

