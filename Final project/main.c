//Nick Dodge, Matthew Herber
// Real Time Embedded Systems CE-426
// Final Project UART Two Person Text Messaging Program

#include "stm32F10x.h"
#include <cmsis_os.h>
#include "uart.h"

#define uart1 0
#define uart2 1
#define MAXMESSAGELENGTH 128

//user transmit and recieve thread definitions
void user1_transmit_thread(void const *argument);
void user1_receive_thread(void const *argument);
void user2_transmit_thread(void const *argument);
void user2_receive_thread(void const *argument);

//user transmit and receive thread definition
osThreadDef(user1_transmit_thread, osPriorityNormal, 1, 0);
osThreadDef(user1_receive_thread, osPriorityNormal, 1, 0);
osThreadDef(user2_transmit_thread, osPriorityNormal, 1, 0);
osThreadDef(user2_receive_thread, osPriorityNormal, 1, 0);

//user thread signal definition
osThreadId u1Transmit;
osThreadId u1Receive;
osThreadId u2Transmit;
osThreadId u2Receive;

// other definitions
void SendText(uint8_t *txt, int USART, osMutexId mutex);
void ClearScreen(int uart, osMutexId mutex);

// Pre Defined Message function definitions
void pre_defined_message1(void const *argument);
void pre_defined_message2(void const *argument);

// Pre Defined Message thread definition
osTimerDef(messageTimer1, pre_defined_message1);
osTimerDef(messageTimer2, pre_defined_message2);
osTimerId preDefinedMessage1ID;
osTimerId preDefinedMessage2ID;

//mail format definition
typedef struct {
	char message[128];
} mail_format;

//mailbox definition with 16 slots
osMailQDef(mail_box, 16, mail_format);
osMailQId mail_box;

mail_format *mail_TX;
mail_format *mail_RX;

//user message creation with 128 characters
char msg1[128];
int msg1_index = 0;
char msg2[128];
int msg2_index = 0;

//user1 message queue definition
osMessageQId uart1_message_queue;
osMessageQDef(uart1_message_queue, 128, unsigned char);
osEvent uart1_msg;

//user2 message queue definition
osMessageQId uart2_message_queue;
osMessageQDef(uart2_message_queue, 128, unsigned char);
osEvent uart2_msg;

// mutex uart1
osMutexId uart1_mutex;
osMutexDef(uart1_mutex);

//mutex uart2
osMutexId uart2_mutex;
osMutexDef(uart2_mutex);

// pre defined message function uart 1
void pre_defined_message1(void const *argument) {
	// send predefined messages
	SendText("[SYSTEM] Hello User 1\n",uart1, uart1_mutex);
}

// pre defined message function uart 2
void pre_defined_message2(void const *argument) {
	// send predefined messages
	SendText("[SYSTEM] Hello User 2\n",uart2, uart2_mutex);
}

void user1_transmit_thread(void const *argument) {
	for(;;) {
		osSignalWait(0x01, osWaitForever); //wait for signal
		uart1_msg = osMessageGet(uart1_message_queue, osWaitForever); //grab message
		if (uart1_msg.value.v == 0xD) { //if ENTER key, send mail
			mail_TX = (mail_format*)osMailAlloc(mail_box, osWaitForever);
			int i;
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				mail_TX -> message[i] = msg1[i]; 
			}
			SendChar('\n',uart1);
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				msg1[i] = '\0'; //reset all characters to empty so messages don't repeat
			}
			//release the uart mutex now that we are done with it (user pressed enter)
			osMutexRelease(uart1_mutex);
			msg1_index = 0; //reset message index so we insert new mail at correct spot
			osMailPut(mail_box, mail_TX); //send mail
			osSignalSet(u2Receive, 0x01); //set signal to user2 recieve to let thread know we have incoming mail
		} else { //if not ENTER
			if (msg1_index == 0) // if user starts typing
			{
				//wait for uart mutex to open, and take control of the mutex once it is
				osMutexWait(uart1_mutex, osWaitForever);
			}
			if (msg1_index > MAXMESSAGELENGTH) {
				char warnStr[52] = "[Warning] Message too long. Sending message to user";
				SendChar('\n',uart1);
				int i;
				for (i = 0; i < 52; i++) {
								SendChar(warnStr[i],uart1); //send warning string
				}
				SendChar('\n',uart1);
		
				//send mail
				mail_TX = (mail_format*)osMailAlloc(mail_box, osWaitForever);
				for (i = 0; i < MAXMESSAGELENGTH; i++) {
					mail_TX -> message[i] = msg1[i];
				}
				SendChar('\n',uart1);
				for (i = 0; i < MAXMESSAGELENGTH; i++) {
					msg1[i] = '\0'; //reset all characters to empty so messages don't repeat
				}
				osMutexRelease(uart1_mutex); //release the uart mutex now that we are done with it (message too long)
				msg1_index = 0; //reset message index so we insert new mail at correct spot
				osMailPut(mail_box, mail_TX); //send mail
				osSignalSet(u2Receive, 0x01); //set signal to user2 recieve to let thread know we have incoming mail
			} else {
				SendChar(uart1_msg.value.v, uart1);
				msg1[msg1_index] = uart1_msg.value.v; //add character to mail value
				msg1_index++; //increase index for mail
			}
		}
	}
}

void user1_receive_thread(void const *argument) {
	for(;;) {
		osSignalWait(0x01, osWaitForever); //wait for signal
		osEvent evt = osMailGet(mail_box, osWaitForever); //get latest mail
		if (evt.status == osEventMail){ //if actual mail
			osMutexWait(uart1_mutex, osWaitForever);
			mail_RX = (mail_format*)evt.value.p;
			char usrStr[9] = "[User 2] "; //start off message with identifier
			int i;
			for (i = 0; i < 9; i++) {
				SendChar(usrStr[i],uart1);
			}
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				if (mail_RX->message[i] == '\0') {
					SendChar('\n',uart1);
					break;
				} else {
					SendChar(mail_RX->message[i],uart1);
				}
			}
			osMutexRelease(uart1_mutex);
			osMailFree(mail_box, mail_RX); //remove mail from mail queue
		}
	}
}

void user2_transmit_thread(void const *argument) {
	for(;;) {
		osSignalWait(0x01, osWaitForever); //wait for signal
		uart2_msg = osMessageGet(uart2_message_queue, osWaitForever); //grab message
		if (uart2_msg.value.v == 0xD) { //if character is ENTER key, send mail
			mail_TX = (mail_format*)osMailAlloc(mail_box, osWaitForever); 
			int i;
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				mail_TX -> message[i] = msg2[i];
			}
			SendChar('\n',uart2);
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				msg2[i] = '\0'; //reset all characters to empty so messages don't repeat
			}
			osMutexRelease(uart2_mutex); //release the uart mutex now that we are done with it (user pressed enter)
			msg2_index = 0; //reset message index so we insert new mail at correct spot
			osMailPut(mail_box, mail_TX); //send mail
			osSignalSet(u1Receive, 0x01); //set signal to user1recieve to let thread know we have incoming mail
		} else {
			if (msg2_index == 0) // if user starts typing
			{
			//wait for uart mutex to open, and take control of the mutex once it is
			osMutexWait(uart2_mutex, osWaitForever);
			}
			if (msg2_index > MAXMESSAGELENGTH) {
				char warnStr[52] = "[Warning] Message too long. Sending message to user";
				SendChar('\n',uart2);
				int i;
				for (i = 0; i < 52; i++) {
								SendChar(warnStr[i],uart2); //send warning string
				}
				SendChar('\n',uart2);
				//send mail
				mail_TX = (mail_format*)osMailAlloc(mail_box, osWaitForever);
				for (i = 0; i < MAXMESSAGELENGTH; i++) {
					mail_TX -> message[i] = msg2[i]; 
				}
				SendChar('\n',uart2); //new line character
				for (i = 0; i < MAXMESSAGELENGTH; i++) {
					msg2[i] = '\0'; //reset all characters to empty so messages don't repeat
				}
				osMutexRelease(uart2_mutex); //release the uart mutex now that we are done with it (message too long)
				msg2_index = 0; //reset message index so we insert new mail at correct spot
				osMailPut(mail_box, mail_TX); //send mail
				osSignalSet(u1Receive, 0x01); //set signal to user2 recieve to let thread know we have incoming mail
			} else {
				SendChar(uart2_msg.value.v, uart2);
				msg2[msg2_index] = uart2_msg.value.v; //add character to mail value
				msg2_index++; //increase index for mail
			}
		}
	}
}

void user2_receive_thread(void const *argument) {
	for(;;) {
		osSignalWait(0x01, osWaitForever); //wait for signal
		osEvent evt = osMailGet(mail_box, osWaitForever); //get latest mail
		if (evt.status == osEventMail){ //if actual mail
			osMutexWait(uart2_mutex, osWaitForever);
			mail_RX = (mail_format*)evt.value.p;
			char usrStr[9] = "[User 1] "; //start off message with identifier
			int i;
			for (i = 0; i < 9; i++) {
				SendChar(usrStr[i],uart2);
			}			
			for (i = 0; i < MAXMESSAGELENGTH; i++) {
				if (mail_RX->message[i] == '\0') {
					SendChar('\n',uart2); //if end of message, print newline char
					break;
				} else {
					SendChar(mail_RX->message[i],uart2);
				}
			}
			osMutexRelease(uart2_mutex);
			osMailFree(mail_box, mail_RX); //remove mail from mail queue
		}
	}
}

int main(void) {
	osKernelInitialize();
	
	//uart1 initialization
	USART1_Init();
	NVIC->ICPR[USART1_IRQn/32] = 1UL << (USART1_IRQn%32); //clear any previous pending interrupt flag
	NVIC->IP[USART1_IRQn] = 0x80; //set priority to 0x80
	NVIC->ISER[USART1_IRQn/32] = 1UL << (USART1_IRQn%32); //set interrupt enable bit
	USART1->CR1 |= USART_CR1_RXNEIE; //enable USART receiver not empty interrupt
	
	//uart2 initialization
	USART2_Init();
	NVIC->ICPR[USART2_IRQn/32] = 1UL << (USART2_IRQn%32); //clear any previous pending interrupt flag
	NVIC->IP[USART2_IRQn] = 0x80; //set priority to 0x80
  NVIC->ISER[USART2_IRQn/32] = 1UL << (USART2_IRQn%32); //set interrupt enable bit
  USART2->CR1 |= USART_CR1_RXNEIE; //enable USART receiver not empty interrupt
	
	//create mutexs
	uart1_mutex = osMutexCreate(osMutex(uart1_mutex));
	uart2_mutex = osMutexCreate(osMutex(uart2_mutex));
	
	//mail box and message queue creation
	mail_box = osMailCreate(osMailQ(mail_box), NULL);
	uart1_message_queue = osMessageCreate(osMessageQ(uart1_message_queue), NULL);
	uart2_message_queue = osMessageCreate(osMessageQ(uart2_message_queue), NULL);
	
	//thread creation
	u1Transmit = osThreadCreate(osThread(user1_transmit_thread), NULL);
	u1Receive = osThreadCreate(osThread(user1_receive_thread), NULL);
	u2Transmit = osThreadCreate(osThread(user2_transmit_thread), NULL);
	u2Receive = osThreadCreate(osThread(user2_receive_thread), NULL);
	
	//pre defined message timer start
	preDefinedMessage1ID = osTimerCreate(osTimer(messageTimer1), osTimerPeriodic, (void*)0);
	preDefinedMessage2ID = osTimerCreate(osTimer(messageTimer2), osTimerPeriodic, (void*)0);
	osTimerStart(preDefinedMessage1ID, 50000);
	osTimerStart(preDefinedMessage2ID, 80000);
	
	osKernelStart();
}

// uart interupt handlers
void USART1_IRQHandler (void) {
  uint8_t intKey = (int8_t)(USART1->DR & 0x1FF);
  osMessagePut(uart1_message_queue, intKey, 0); //add message into message queue
	osSignalSet(u1Transmit, 0x01);
}

void USART2_IRQHandler (void) {
  uint8_t intKey = (int8_t) (USART2->DR & 0x1FF);
  osMessagePut(uart2_message_queue, intKey, 0); //add message into message queue
	osSignalSet(u2Transmit, 0x01);
}

void SendText(uint8_t *text, int USART, osMutexId mutex) {
	//wait for uart mutex to open, and take control of the mutex once it is
	osMutexWait(mutex, 10000);
	
	uint32_t index = 0;
	
	//run until we hit the null character indicating the end of the string
	while(text[index] != '\0')
	{
		SendChar(text[index], USART);
		index++;
	}
	
	//release the uart mutex now that we are done with it
	osMutexRelease(mutex);
}

void ClearScreen(int uart, osMutexId mutex)
{
	SendText("\033[2J", uart, mutex);
}
