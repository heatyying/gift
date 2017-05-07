#ifdef GLOBALS 
#define EXT
#else
#define EXT extern 
#endif

#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include "misc.h"
#include "stm32f10x_gpio.h"	   
#include "stm32f10x_usart.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_sdio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"

EXT __IO uint32_t TimingDelay;

EXT void VS1003_SetVolume(unsigned char  leftchannel,unsigned char rightchannel);
EXT void VS1003_SoftReset(void);
EXT void VS1003_WriteReg(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte);
EXT unsigned int VS1003_ReadReg(unsigned char addressbyte);

EXT unsigned char VS1003_ReadByte(void);
EXT unsigned char VS1003_WriteByte(u8 byte);
EXT void VS1003_start(void);
EXT void SPI_VS1003_Init(void);
EXT void VS1003_SetVol(unsigned char vol);


#define Led_ON()   GPIO_SetBits(GPIOB, GPIO_Pin_5);  	      //LED1ÁÁ
#define Led_OFF()  GPIO_ResetBits(GPIOB, GPIO_Pin_5); 	      //LED2Ãð
#define _DF1S	0x81




