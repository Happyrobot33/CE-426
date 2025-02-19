/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Signal Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <cmsis_os.h>
#include "Board_LED.h"

void green_thread (void const *argument);
void yellow_thread (void const *argument);
void red_thread (void const *argument);

osThreadDef(green_thread, osPriorityNormal, 1, 0);
osThreadDef(yellow_thread, osPriorityNormal, 1, 0);
osThreadDef(red_thread, osPriorityNormal, 1, 0);

osThreadId T_led_ID_green;
osThreadId T_led_ID_yellow;
osThreadId T_led_ID_red;

#define GREEN_LED 2
#define YELLOW_LED 1
#define RED_LED 0

#define FLAG 0x01

void delay (void)
{
unsigned int index;
const unsigned int count = 1000000;
	for(index =0;index<count;index++)
	{
		;
	}
}

/*----------------------------------------------------------------------------
  Flash LED 1 when signaled by the led_Thread2
 *---------------------------------------------------------------------------*/
void green_thread (void const *argument) 
{
	for (;;) 
	{
		//wait for flag signal to occur
		osSignalWait(FLAG, osWaitForever);
		
		//Control the LED for the correct ammount of time
		LED_On(GREEN_LED);
		osDelay(3000);
		LED_Off(GREEN_LED);
		
		//send out signal to yellow
		osSignalSet(T_led_ID_yellow, FLAG);
	}
}
/*----------------------------------------------------------------------------
  Flash LED two and synchronise the flashing of LED 1 by setting a signal flag
 *---------------------------------------------------------------------------*/
void yellow_thread (void const *argument) 
{
	for (;;) 
	{
		//wait for flag signal to occur
		osSignalWait(FLAG, osWaitForever);
		
		//Control the LED for the correct ammount of time
		LED_On(YELLOW_LED);
		osDelay(1000);
		LED_Off(YELLOW_LED);
		
		//send out signal to red
		osSignalSet(T_led_ID_red, FLAG);
	}
}

void red_thread (void const *argument) 
{
	for (;;) 
	{
		//wait for flag signal to occur
		osSignalWait(FLAG, osWaitForever);
		
		//Control the LED for the correct ammount of time
		LED_On(RED_LED);
		osDelay(3000);
		LED_Off(RED_LED);
		
		//send out signal to green
		osSignalSet(T_led_ID_green, FLAG);
	}
}



/*----------------------------------------------------------------------------
  Start the threads
 *---------------------------------------------------------------------------*/
int main (void) 
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	LED_Initialize();
	T_led_ID_green = osThreadCreate(osThread(green_thread), NULL);
	T_led_ID_yellow = osThreadCreate(osThread(yellow_thread), NULL);
	T_led_ID_red = osThreadCreate(osThread(red_thread), NULL);
	
	//set the flag to start green
	osSignalSet(T_led_ID_green, FLAG);
	
	osKernelStart ();                         // start thread execution 
}