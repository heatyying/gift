/****************************************************************************
* Copyright (C), 2013 奋斗嵌入式工作室 www.ourstm.net
*
* 本例程在 奋斗版STM32开发板V3，V5上调试通过           
* QQ: 9191274, 旺旺：sun68, Email: sun68@163.com 
* 淘宝店铺：ourstm.taobao.com  
*
* 文件名: VS1003.c
* 内容简述:
*       本例程包含了VS1003的底层驱动函数
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.2    2011-7-28 sun68  创建该文件
*
*/
#include "vs1003.h"   
#include "stm32f10x_spi.h"
unsigned char  BassEnhanceValue=0x00,TrebleEnhanceValue=0x00;
extern void Delay_us(__IO uint32_t nTime);

/****************************************************************************
* 名    称：void SPI_VS1003_Init(void)
* 功    能：VS1003初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void SPI_VS1003_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
   
  /* 使能SPI2 时钟 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 ,ENABLE);	
  /* 配置 SPI2 引脚: SCK, MISO 和 MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  //V3---BRF24L01的SPI2 片选 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
 
  /* 配置PB12为VS1003B的片选  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);   
 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;    //PE0 VS1003 RST   
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;    //PC6 VS1003 XDCS  串行数据接口选择  
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;	 //PC7 VS1003 DREQ
  GPIO_Init(GPIOC, &GPIO_InitStructure);

 
  /* SPI2 configuration */ 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;				  			//SPI 主模式
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;			  			//8位 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;				  			//平时CLK 为低
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;  //由于VS1003的响应速度，SPI速度还不能太快
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //高位在前
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);
  
  /* Enable SPI2  */
  SPI_Cmd(SPI2, ENABLE);   									  		  //使能SPI2 

  GPIO_SetBits(GPIOB, GPIO_Pin_0);									  //禁止NRF24L01的片选
  
}

/****************************************************************************
* 名    称：u8 VS1003_ReadByte(void)
* 功    能：从VS1003读出一个字节
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
u8 VS1003_ReadByte(void)
{
   while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);  	   //判断SPI2 发送缓冲区是否空  
   SPI_I2S_SendData(SPI2, 0);										   //发送一个空字节	   
   while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);	   //判断是否接收缓冲区非空
   return SPI_I2S_ReceiveData(SPI2);								   //返回接收到的数据
}

/****************************************************************************
* 名    称：u8 VS1003_WriteByte(u8 byte)
* 功    能：向VS1003写入一个字节
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
u8 VS1003_WriteByte(u8 byte)
{			  
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);		//判断SPI2 发送缓冲区是否空  
  SPI_I2S_SendData(SPI2, byte);											//发送8位的数据
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);	    //判断是否接收缓冲区非空
  return SPI_I2S_ReceiveData(SPI2);										//返回接收到的数据
}


/****************************************************************************
* 名    称：void VS1003_WriteReg(unsigned char add, unsigned char highbyte, unsigned char lowbyte)
* 功    能：写vs1003寄存器
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void VS1003_WriteReg(unsigned char add, unsigned char highbyte, unsigned char lowbyte)
{
	XDCS_SET(1);                    			//xDCS = 1
	CS_SET(0);                      			//xCS = 0
	VS1003_WriteByte(VS_WRITE_COMMAND); 		//发送写寄存器命令
	VS1003_WriteByte(add);      				//发送寄存器的地址
	VS1003_WriteByte(highbyte);         		//发送待写数据的高8位
	VS1003_WriteByte(lowbyte);          		//发送待写数据的低8位
	CS_SET(1);       							//xCS = 1
}


/****************************************************************************
* 名    称：unsigned int VS1003_ReadReg(unsigned char add)
* 功    能：读vs1003寄存器
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
unsigned int VS1003_ReadReg(unsigned char add)
{
	unsigned int resultvalue = 0;
	XDCS_SET(1);                			//xDCS =1
	CS_SET(0);                   			//xCS = 0
	VS1003_WriteByte(VS_READ_COMMAND); 		//发送读寄存器命令
	VS1003_WriteByte((add));				//发送寄存器的地址
	resultvalue = (unsigned int )(VS1003_ReadByte() << 8);//读取高8位数据
	resultvalue |= VS1003_ReadByte();  		//读取低8位数据
	CS_SET(1);                    			//xCS = 1        
	return resultvalue;                 	//返回16位寄存器的值
}

/****************************************************************************
* 名    称：void VS1003_start(void)
* 功    能：VS1003的初始设置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void VS1003_start(void)
{
	RST_SET(0);          					//xRST = 0   复位vs1003(); 
	Delay_us(1000);       					//延时10ms
	VS1003_WriteByte(0xff);   				//发送一个字节的无效数据，启动SPI传输
	XDCS_SET(1);         					//xDCS =1
	CS_SET(1);           					//xCS = 1
	RST_SET(1);          					//xRST =1   
	Delay_us(1000);             			//延时10ms
 	VS1003_WriteReg(SPI_MODE,0x08,0x00);  	//进入vs1003的播放模式
	VS1003_WriteReg(3, 0x98, 0x00);   		//设置vs1003的时钟,3倍频
	VS1003_WriteReg(5, 0xBB, 0x81);   		//采样率48k，立体声
	VS1003_WriteReg(SPI_BASS, TrebleEnhanceValue, BassEnhanceValue);//设置重音
	VS1003_WriteReg(0x0b,0x00,0x00);      	//VS1003 音量
	while(DREQ==0);						   	//等待DREQ为高  表示能够接受音乐数据输入
}
/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/

