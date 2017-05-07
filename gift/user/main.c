/****************************************************************************
* Copyright (C), 2013 奋斗嵌入式工作室 www.ourstm.net
*
* 本例程在 奋斗版STM32开发板V3,V5上调试通过           
* QQ: 9191274, 旺旺：sun68, Email: sun68@163.com 
* 淘宝店铺：ourstm.taobao.com  
*
* 文件名: main.c
* 内容简述:	
*       
*	TF卡上的的Tini-FatFs0.09a实现，测试了将TF卡根目录下的音乐类型文件内容通过VS1003
    播放出来，支持长文件名， 支持中文文件名，支持的文件类型为：MP3，MID，WAV WMA
*	辅助软件：PC机上需要运行串口调试助手软件。
    
	基于MDK版本：        3.8
	基于官方外设库版本： 3.5
	基于Tiny-FatFs版本： 0.09A
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.1    2011-8-17 sun68  创建该文件 文件系统FATFS0.08b
* V0.2    2013.1.28 sun68  升级为文件系统FATFS0.09a
                           解决了MDK4编译会造成文件名比对错误的问题。 
*/
/* Includes ------------------------------------------------------------------*/

#define GLOBALS 

#include "vs1003.h"	 
#include "sdio_sdcard.h"
#include "ff.h"
#include "ffconf.h"
#include "diskio.h"
#include "demo.h"


/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;

SD_CardInfo SDCardInfo;
uint32_t Buffer_Block_Tx[512], Buffer_Block_Rx[512]; 
SD_Error Status = SD_OK;


void RCC_Configuration(void);
void NVIC_Configuration(void);
void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
void Usart1_Init(void);	;
void Delay_us(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

		
FATFS fs;                      // 逻辑驱动器的标志
FIL fsrc, fdst;                // 文件标志 
unsigned char buffer[512];     // 文件内容缓冲区
FRESULT res;                   // FatFs 功能函数返回结果变量
unsigned int br, bw;           // 文件读/写计数器


/****************************************************************************
* 名    称：void Delay_us(__IO uint32_t nTime)
* 功    能：定时延时程序 10us为单位
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/  
void Delay_us(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}
/****************************************************************************
* 名    称：void TimingDelay_Decrement(void)
* 功    能：获取节拍程序
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/  
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/****************************************************************************
* 名    称：void RCC_Configuration(void)
* 功    能：系统时钟配置为72MHZ， 外设时钟配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void RCC_Configuration(void){

  SystemInit(); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC 
  						| RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOE , ENABLE);
}
/****************************************************************************
* 名    称：void SD_TEST(void)
* 功    能：SD卡测试函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void SD_TEST(void){
	Status = SD_Init();
    Status = SD_GetCardInfo(&SDCardInfo);
    Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
    Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
    Status = SD_SetDeviceMode(SD_DMA_MODE);  
 	if (Status == SD_OK)
  	{
        // 从地址0开始读取512字节  
    	Status = SD_ReadBlock(Buffer_Block_Rx, 0x00,  512); 
  	}
  	if (Status == SD_OK)
    {	 
       // 返回成功的话，串口输出SD卡测试成功信息 
	    USART_OUT(USART1,"\r\nSD SDIO-4bit模式 测试TF卡成功！ \n ");
    } 
}
/*-----------例程所用到的文件系统函数原型------------------------

---------------在文件系统中注册/注销 工作区（0-9)-----------------------------------------------------------------
FRESULT f_mount (
  BYTE  Drive,                 	//Logical drive number 
  FATFS*  FileSystemObject  	// Pointer to the work area 
);
参数：
Drive：用逻辑驱动器号(0-9) 去注册/注销 工作区 
FileSystemObject：文件系统工作区的标识。 
--------------------------------------------------------------------------------

---------------打开目录------------------------------------------------------------------
FRESULT f_opendir (
  DIR* DirObject,       	//空目录结构指针
  const TCHAR* DirName  	//目录名指针
)
返回值：
FR_OK (0)  打开目录成功
--------------------------------------------------------------------------------
---------------读取目录下的文件名 --------------------------------------------------------
FRESULT f_readdir (
  DIR* DirObject,             // 目录结构指针
  FILINFO* FileInfo           // 文件信息指针 
);
返回值：
FR_OK (0)  读取文件名成功
-------------------------------------------------------------------------------------------
----------------为了存取数据，打开文件目标----------------------------------------------------------
FRESULT f_open (
  FIL* FileObject,       	 // 文件结构型指针 
  const TCHAR* FileName, 	 // 文件名指针
  BYTE ModeFlags             // 模式标志 
);
ModeFlags：
FA_OPEN_EXISTING： 指定为文件不存在的话，返回错误
FA_READ：          指定为文件读
FA_CREATE_ALWAYS： 文件不存在的话，直接建立
FA_WRITE: 		   指定为文件写

--------------------------------------------------------------------------
-----------------读文件数据--------------------------------------------------------------
FRESULT f_read (
  FIL* FileObject,    // 文件结构型指针 
  void* Buffer,       // 读数据缓冲区指针
  UINT ByteToRead,    // 要读取的字节数 
  UINT* ByteRead      // 返回的所读取的可变字节数的指针 
);
----------------------------------------------------------------------------------

-----------------写文件数据-------------------------------------------------------
FRESULT f_write (
  FIL* FileObject,     // 文件结构型指针 
  const void* Buffer,  // 写数据缓冲区指针
  UINT ByteToWrite,    // 要写入的字节数 
  UINT* ByteWritten    // 返回的所写入的可变字节数的指针 
);
----------------------------------------------------------------------------------
-----------------关闭打开的文件---------------------------------------------------
FRESULT f_close (
  FIL* FileObject     // 文件结构型指针 
);

-------------------------------------------------------------------------------
*/ 

/****************************************************************************
* 名    称：void Play_Music(void)
* 功    能：TXT文件输出函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Play_Music(void)
{ 
  FILINFO finfo;
  DIR dirs;
  unsigned int count=0;
  char path[50]={""},j=0;  
  char *result1,*result2,*result3,*result4; 
  unsigned int volcnt=0;

  unsigned char vol=0;
  unsigned int volcntmax=10000;
  unsigned int voldest=0;
  

  /*开启长文件名功能时， 要预先初始化文件名缓冲区的长度 */
  #if _USE_LFN
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
    finfo.lfname = lfn;
    finfo.lfsize = sizeof(lfn);
  #endif
  USART_OUT(USART1,"\n文件系统(Tini-FatFS0.09a)启动成功! \n");
  
  disk_initialize(0);						     //fatfs可以管理多个介质的分区， 所以把物理存储介质SST25VF016B标示为0区，相当于磁盘编号
  VS1003_start();
  f_mount(0, &fs);							     //将文件系统设置到0区 
  if (f_opendir(&dirs, path) == FR_OK)            //读取该磁盘的根目录        
  {
    while (f_readdir(&dirs, &finfo) == FR_OK)  	 //循环依次读取文件名
    {	 
      if (finfo.fattrib & AM_ARC) 			     //判断文件属性是否为存档型	 TXT文件一般都为存档型
      {
        if(!finfo.fname[0])	 break;    		     //如果是文件名为空表示到目录的末尾。退出	 
		if(finfo.lfname[0]){	 		         //长文件名	
			USART_OUT(USART1,"\r\n文件名是:\n   %s\n",finfo.lfname);	         //输出文件名
			result1=strstr(finfo.lfname,".mp3");	     //比较后缀是否为音频文件	
			result2=strstr(finfo.lfname,".mid");	     //比较后缀是否为音频文件	
			result3=strstr(finfo.lfname,".wav");	     //比较后缀是否为音频文件	
			result4=strstr(finfo.lfname,".wma");	     //比较后缀是否为音频文件
		}
		else{									  //8.3格式文件名
			USART_OUT(USART1,"\r\n文件名是:\n   %s\n",finfo.fname);	         //输出文件名
			result1=strstr(finfo.fname,".mp3");	     //比较后缀是否为音频文件	
			result2=strstr(finfo.fname,".mid");	     //比较后缀是否为音频文件	
			result3=strstr(finfo.fname,".wav");	     //比较后缀是否为音频文件	
			result4=strstr(finfo.fname,".wma");	     //比较后缀是否为音频文件
		}	
		if(result1!=NULL||result2!=NULL||result3!=NULL||result4!=NULL){
        	if(finfo.lfname[0]){	 		         //长文件名	
				res = f_open(&fsrc, finfo.lfname, FA_OPEN_EXISTING | FA_READ);   
			}
			else{									 //8.3格式文件名
				res = f_open(&fsrc, finfo.fname, FA_OPEN_EXISTING | FA_READ);    
			}
			USART_OUT(USART1,"\r\n Playing...");
       		br=1; 
			XDCS_SET(0);                         //xDCS = 0，选择vs1003的数据接口
			for (;;) { 		
    	    	res = f_read(&fsrc, buffer, sizeof(buffer), &br);	  
            	if(res==0){					
					count=0;
					Delay_us(1000);
					while(count<512){ 						   //根据SD卡介质的原因，文件每次只能读出512字节														   
            			if(DREQ!=0){						   //VS1003数据申请口线， 每次为高后，可以通过SPI2写入32字节的音频数据						
 							for(j=0;j<32;j++)				   //每次送32个数据
							{
								VS1003_WriteByte(buffer[count]); //写入音频数据								
								count++;
							}

							if (volcnt ==0 )
								{voldest = 1;}

							if (volcnt == volcntmax)
								{voldest = 0;}
							if (voldest ==1)
							{volcnt++;}
							if (voldest == 0)
								{volcnt--;}
							vol =(unsigned char)(volcnt/200);

							if (volcnt % 1== 0)
							{ 
								VS1003_SetVol(vol); 
							}

							
						}
    	    			if (res || br == 0) break;   // 是否到文件尾
					}
       			}
				if (res || br == 0) break;   // 是否到文件尾		   
        	}
			count=0;
			while(count<2048){	             //根据VS1003的特性，需要音乐文件的末尾发送一些个0	，保证下一个音频文件的播放
       	     if(DREQ!=0){
 				for(j=0;j<32;j++)            //每次送32个数据
				{
					VS1003_WriteByte(0); 						
					count++;
				}
			}
		}
		XDCS_SET(1);    
		f_close(&fsrc);
      }
    } 
   } 
  } 
} 


/****************************************************************************
* 名    称：int main(void)
* 功    能：主函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
int main(void)
{
	RCC_Configuration();	             //设置内部时钟及外设时钟使能
	if (SysTick_Config(720))		     //时钟节拍中断时10us一次  用于定时 
	{ 
	/* Capture error */ 
	while (1);
	}     
	NVIC_Configuration();			     //中断源配置  
	     					//xRST =1   

	Usart1_Init();		             //串口1初始化
	SPI_VS1003_Init();				 //VS1003 初始化    


	Delay_us(10000);					 //延时100ms
	SD_TEST();                         //SD卡测试函数

	while (1)
	{
		Play_Music();						 //播放音频
	}
  
}
/****************************************************************************
* 名    称：void Usart1_Init(void)
* 功    能：串口1初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Usart1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 			      //LCD背光控制
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOD, GPIO_Pin_13);			                  //LCD背光关闭	


  RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 , ENABLE);	 //使能串口1时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         		 //USART1 TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    		 //复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);		    		 //A端口 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	         	 //USART1 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   	 //复用开漏输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);		         	 //A端口 

  USART_InitStructure.USART_BaudRate = 115200;						//速率115200bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;			//停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);							//配置串口参数函数   
   /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);	
  
}


/****************************************************************************
* 名    称：void NVIC_Configuration(void)
* 功    能：中断源配置函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* 优先级组1 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	  

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;			     //SDIO中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		 //抢先优先级0  范围：0或1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			 //子优先级0	范围：0-7
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/******************************************************
		整形数据转字符串函数
        char *itoa(int value, char *string, int radix)
		radix=10 标示是10进制	非十进制，转换结果为0;  

	    例：d=-379;
		执行	itoa(d, buf, 10); 后
		
		buf="-379"							   			  
**********************************************************/
char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */
/****************************************************************************
* 名    称：void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...)
* 功    能：格式化串口输出函数
* 入口参数：USARTx:  指定串口
			Data：   发送数组
			...:     不定参数
* 出口参数：无
* 说    明：格式化串口输出函数
        	"\r"	回车符	   USART_OUT(USART1, "abcdefg\r")   
			"\n"	换行符	   USART_OUT(USART1, "abcdefg\r\n")
			"%s"	字符串	   USART_OUT(USART1, "字符串是：%s","abcdefg")
			"%d"	十进制	   USART_OUT(USART1, "a=%d",10)
* 调用方法：无 
****************************************************************************/
void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...){ 

	const char *s;
    int d;
   
    char buf[16];
    va_list ap;
    va_start(ap, Data);

	while(*Data!=0){				                          //判断是否到达字符串结束符
		if(*Data==0x5c){									  //'\'
			switch (*++Data){
				case 'r':							          //回车符
					USART_SendData(USARTx, 0x0d);	   

					Data++;
					break;
				case 'n':							          //换行符
					USART_SendData(USARTx, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				    break;
			}
			
			 
		}
		else if(*Data=='%'){									  //
			switch (*++Data){				
				case 's':										  //字符串
                	s = va_arg(ap, const char *);
                	for ( ; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
            	case 'd':										  //十进制
                	d = va_arg(ap, int);
                	itoa(d, buf, 10);
                	for (s = buf; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				default:
					Data++;
				    break;
			}		 
		}
		else USART_SendData(USARTx, *Data++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}
/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/

