/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Mutex Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"
#include "Board_LED.h"

osMessageQId CHARACTER_Q;		//define the message queue
osMessageQDef(CHARACTER_Q, 0x16, uint8_t);

void normal_mode_thread (void const *argument);
void override_mode_thread (void const *argument);
void mode_manager_thread (void const *argument);

void SetLights(_Bool green, _Bool yellow, _Bool red);
void UNSAFE_LED_SET(uint32_t num, _Bool val);

void SendText(uint8_t *txt);

osThreadDef(normal_mode_thread, osPriorityNormal, 1, 0);
osThreadDef(override_mode_thread, osPriorityNormal, 1, 0);
osThreadDef(mode_manager_thread, osPriorityNormal, 1, 0);

osThreadId T_led_ID_normal;
osThreadId T_led_ID_override;
osThreadId T_led_ID_manager;

#define GREEN_LED 2
#define YELLOW_LED 1
#define RED_LED 0

#define RUNFLAG 0x01
	
osMutexId uart_mutex;
osMutexDef(uart_mutex);

osMutexId led_mutex;
osMutexDef(led_mutex);

void normal_mode_thread (void const *argument) 
{
	while(1)
	{
		//wait for flag signal to occur
		osSignalWait(RUNFLAG, osWaitForever);
		//reset the signal ourselves to ensure we keep running
		osSignalSet(T_led_ID_normal, RUNFLAG);
		
		//wait for mutex to be clear on all 3
		osMutexWait(led_mutex, osWaitForever);
		
		//Control the LED for the correct ammount of time
		SetLights(1, 0, 0);
		osDelay(1000);
		SetLights(0, 1, 0);
		osDelay(250);
		SetLights(0, 0, 1);
		osDelay(1000);
		
		osMutexRelease(led_mutex);
	}
}

void override_mode_thread (void const *argument) 
{
	while(1)
	{
		//wait for flag signal to occur
		osSignalWait(RUNFLAG, osWaitForever);
		//reset the signal ourselves to ensure we keep running
		osSignalSet(T_led_ID_override, RUNFLAG);
		
		//wait for mutex to be clear on all 3
		osMutexWait(led_mutex, osWaitForever);
		
		//Control the LED for the correct ammount of time
		SetLights(1, 0, 0);
		osDelay(250);
		SetLights(0, 0, 0);
		osDelay(250);
		
		osMutexRelease(led_mutex);
	}
}

void mode_manager_thread (void const *argument) 
{
	osEvent result;
	for(;;)
	{
		//Wait for message to arrive
		result = osMessageGet(CHARACTER_Q, osWaitForever);
		
		SendChar(result.value.v);
		
		if(result.value.v == '0')
		{
			osSignalClear(T_led_ID_normal, RUNFLAG);
			osSignalSet(T_led_ID_override, RUNFLAG);
		}
		else if(result.value.v == '1')
		{
			osSignalClear(T_led_ID_override, RUNFLAG);
			osSignalSet(T_led_ID_normal, RUNFLAG);
		}
		else
		{
			osSignalClear(T_led_ID_override, RUNFLAG);
			osSignalClear(T_led_ID_normal, RUNFLAG);
			
			//turn off all 3 lights
			osMutexWait(led_mutex, osWaitForever);
			SetLights(0, 0, 0);
			osMutexRelease(led_mutex);
		}
	}
}

void USART1_IRQHandler (void) {
	uint8_t intKey = (int8_t) (USART1->DR & 0x1FF);
	//Note: intKey is a local variable for temporarily holding the input character
	//add a command here to send the received data through message queue, with zero timeout
	//we use 0 timeout as otherwise if the que is full we will never leave this IRQ
	osStatus stat;
	stat = osMessagePut(CHARACTER_Q, intKey, 0);
	//SendChar(intKey);
}

/*----------------------------------------------------------------------------
  Start the threads
 *---------------------------------------------------------------------------*/
int main (void) 
{
	osKernelInitialize();                    // initialize CMSIS-RTOS
	
	//initialize the UART
	USART1_Init();
	//configure USART interrupt ... so we can read user inputs using interrupt
	//Configure and enable USART1 interrupt
	NVIC->ICPR[USART1_IRQn/32] = 1UL << (USART1_IRQn%32); //clear any previous pending interrupt flag
	NVIC->IP[USART1_IRQn] = 0x80; // NVIC_SetPriority(USART1_IRQn, 0x80); //set priority to 0x80
	NVIC->ISER[USART1_IRQn/32] = 1UL << (USART1_IRQn%32); //set interrupt enable bit
	USART1->CR1 |= USART_CR1_RXNEIE; //enable USART receiver not empty interrupt
	
	//create the message queue
	CHARACTER_Q = osMessageCreate(osMessageQ(CHARACTER_Q),NULL);
	
	//initialize LEDs
	LED_Initialize();
	//setup the mutex object for the uart and LEDs
	uart_mutex = osMutexCreate(osMutex(uart_mutex));
	led_mutex = osMutexCreate(osMutex(led_mutex));
	
	//create threads for the LEDs and get their IDs saved into global variables
	T_led_ID_normal = osThreadCreate(osThread(normal_mode_thread), NULL);
	T_led_ID_override = osThreadCreate(osThread(override_mode_thread), NULL);
	T_led_ID_manager = osThreadCreate(osThread(mode_manager_thread), NULL);
	
	//start thread execution
	osKernelStart ();
}

void UNSAFE_LED_SET(uint32_t num, _Bool val)
{
	if(val)
	{
		LED_On(num);
	}
	else
	{
		LED_Off(num);
	}
}

void SetLights(_Bool green, _Bool yellow, _Bool red)
{
	UNSAFE_LED_SET(GREEN_LED, green);
	UNSAFE_LED_SET(YELLOW_LED, yellow);
	UNSAFE_LED_SET(RED_LED, red);
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