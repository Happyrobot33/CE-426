#include <stm32F10x.h>

extern void USART1_Init (void);
extern void USART2_Init (void);
extern int SendChar (int ch, int USART);
extern int GetKey (int USART);

