#include "stm32f10x.h"


typedef struct {
	unsigned char pressedcnt;
	unsigned char threshold;
	unsigned char isPressed;
	GPIO_TypeDef * keyGPIO;
	unsigned short keyGPIOPin;
} KeyPressedType;

