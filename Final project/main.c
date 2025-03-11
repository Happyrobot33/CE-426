/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Mutex Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"

#define MAXMESSAGELENGTH 128
#define MAXMESSAGEHISTORY 20

osMessageQId CHARACTER_Q_ONE;		//define the message queue
osMessageQDef(CHARACTER_Q_ONE, MAXMESSAGELENGTH, uint8_t);
osMessageQId CHARACTER_Q_TWO;		//define the message queue
osMessageQDef(CHARACTER_Q_TWO, MAXMESSAGELENGTH, uint8_t);

void normal_mode_thread (void const *argument);

void SendText(uint8_t *txt, int USART);
void ClearScreen();

osThreadDef(normal_mode_thread, osPriorityNormal, 1, 0);

osThreadId T_led_ID_normal;
	
osMutexId uart_mutex;
osMutexDef(uart_mutex);

typedef struct {
	char message[MAXMESSAGELENGTH];
	uint32_t sender;
	uint32_t time;
//look into null structs instead	uint8_t terminator; //if non-zero, then this is a termination message instead of being related to a message itself
} message_t;

void normal_mode_thread (void const *argument) 
{
	while(1)
	{
		/*
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
		*/
	}
}

void USART1_IRQHandler (void) {
	uint8_t intKey = (int8_t) (USART1->DR & 0x1FF);
	osStatus stat;
	stat = osMessagePut(CHARACTER_Q_TWO, intKey, 0);
	SendChar(intKey, 0);
}

void USART2_IRQHandler (void) {
  uint8_t intKey = (int8_t) (USART2->DR & 0x1FF);
	osStatus stat;
	stat = osMessagePut(CHARACTER_Q_TWO, intKey, 0);
	SendChar(intKey, 1);
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
	
	USART2_Init ();
	NVIC->ICPR[USART2_IRQn/32] = 1UL << (USART2_IRQn%32); //clear any previous pending interrupt flag
	NVIC->IP[USART2_IRQn] = 0x80; //set priority to 0x80
	NVIC->ISER[USART2_IRQn/32] = 1UL << (USART2_IRQn%32); //set interrupt enable bit
	USART2->CR1 |= USART_CR1_RXNEIE; //enable USART receiver not empty interrupt
	
	//create the message queue
	CHARACTER_Q_ONE = osMessageCreate(osMessageQ(CHARACTER_Q_ONE),NULL);
	CHARACTER_Q_TWO = osMessageCreate(osMessageQ(CHARACTER_Q_TWO),NULL);
	
	//setup the mutex object for the uart and LEDs
	uart_mutex = osMutexCreate(osMutex(uart_mutex));
	
	//create threads for the LEDs and get their IDs saved into global variables
	T_led_ID_normal = osThreadCreate(osThread(normal_mode_thread), NULL);
	
	//start thread execution
	osKernelStart ();
	
	SendText("test", 0);
	ClearScreen();
	SendText("More Test", 0);
}

void ClearScreen()
{
	SendChar(27, 0); //escape character is code 27
	SendText("[2J", 0);
}

void SendText(uint8_t *text, int USART) {
	//wait for uart mutex to open, and take control of the mutex once it is
	osMutexWait(uart_mutex, osWaitForever);
	
	uint32_t index = 0;
	
	//run until we hit the null character indicating the end of the string
	while(text[index] != '\0')
	{
		SendChar(text[index], USART);
		index++;
	}
	
	//release the uart mutex now that we are done with it
	osMutexRelease(uart_mutex);
}