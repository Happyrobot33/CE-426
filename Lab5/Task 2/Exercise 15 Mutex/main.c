/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Mutex Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"
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
	
osMutexId uart_mutex;
osMutexDef(uart_mutex);

void SendText(uint8_t *txt);

/*----------------------------------------------------------------------------
  Flash LED 1 when signaled by the led_Thread2
 *---------------------------------------------------------------------------*/
void green_thread (void const *argument) 
{
	for (;;) 
	{
		SendText("Green Thread Waiting\n");
		
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
		SendText("Yellow Thread Waiting\n");
		
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
		SendText("Red Thread Waiting\n");
		
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
	
	//initialize the UART
	USART1_Init();
	//initialize LEDs
	LED_Initialize();
	//setup the mutex object for the uart
	uart_mutex = osMutexCreate(osMutex(uart_mutex));
	
	//create threads for the LEDs and get their IDs saved into global variables
	T_led_ID_green = osThreadCreate(osThread(green_thread), NULL);
	T_led_ID_yellow = osThreadCreate(osThread(yellow_thread), NULL);
	T_led_ID_red = osThreadCreate(osThread(red_thread), NULL);
	
	//set the flag to start green
	osSignalSet(T_led_ID_green, FLAG);
	
	osKernelStart ();                         // start thread execution 
}

void SendText(uint8_t *text) {
	//wait for uart mutex to open, and take control of the mutex once it is
	osMutexWait(uart_mutex, osWaitForever);
	
	uint32_t index = 0;
	
	//run until we hit the null character indicating the end of the string
	while(text[index] != '\0')
	{
		SendChar(text[index]);
		index++;
	}
	
	//release the uart mutex now that we are done with it
	osMutexRelease(uart_mutex);
}