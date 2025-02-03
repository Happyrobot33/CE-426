/* 
   Sample program 2 – Hello World for Embedded Systems
   Toggle an LED attached to a GPIO pin
	 Use UART inputs to control the operating mode of the program
*/

#include <stm32f10x.h> 
#include "GPIO_STM32F10x.h"
#include "Board_LED.h"
#include "uart.h"

#define  LED_0		0
//this is connected to GPIOB pin 8  (PORTB.8)
#define  LED_1		1
#define  LED_2		2

#define GREEN LED_0
#define YELLOW LED_1
#define RED LED_2

void SetTrafficLight (uint32_t light);
void delay (uint32_t count);
void SendText(uint8_t *txt);

volatile uint8_t intKey;

int main()
{
		LED_Initialize();
		USART1_Init ();
		//Configure and enable USART1 interrupt
		NVIC_SetPriority(USART1_IRQn, 0x80);
		NVIC->ISER[USART1_IRQn/32] = 1UL << (USART1_IRQn%32); //set interrupt enable bit
		//NVIC_EnableIRQ(USART1_IRQn);
		USART1->CR1 |= USART_CR1_RXNEIE; //enable receiver not empty interrupt
	
		//send a character to UART for testing
		SendChar('H');			
		SendChar('i');
		SendChar('\n');			//send newline character
	
		//use your function SendText to send a text string to the UART
		SendText("Welcome!\n");
	
		while (1) {
			//include a command here to echo the input you received back to the terminal window
			SendChar(intKey);
			
			if (intKey == '0')
			{
				//Override mode
				delay(1000);
				SetTrafficLight(RED);
				delay(1000);
				LED_Off(RED);
			}
			else
			{
				//Normal mode
				SetTrafficLight(GREEN);
				delay(3000);
				SetTrafficLight(YELLOW);
				delay(1000);
				SetTrafficLight(RED);
				delay(3000);
			}
		}
}	

void SetTrafficLight(uint32_t light)
{
	LED_Off(GREEN);
	LED_Off(YELLOW);
	LED_Off(RED);
	
	LED_On(light);
}

/*-----------------------------------------------------------------------------
 USART1 IRQ Handler
 The hardware automatically clears the interrupt flag, once the ISR is entered
*----------------------------------------------------------------------------*/
void USART1_IRQHandler (void) {
	intKey = (int16_t) (USART1->DR & 0x1FF);
}


/*----------------------------------------------------------------------------
  Simple delay routine
*---------------------------------------------------------------------------*/
void delay (uint32_t count)
{
	uint32_t index1, index2;

	for(index1 =0; index1 < count; index1++)
	{
			for (index2 = 0; index2 < 1000; index2++)
				__NOP();	
	}
}

//complete this function for sending a string of characters to the UART
void SendText(uint8_t *text) {
	uint32_t index = 0;
	
	while(text[index] != '\0')
	{
		SendChar(text[index]);
		index++;
	}
}
