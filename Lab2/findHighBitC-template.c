#include <stm32f10x.h>

uint32_t findHighBitC(uint32_t* array, uint32_t nelements)
{
	uint32_t pos = (32*nelements)-1;	//initialize position
	uint32_t curr;						//current array element to process
	uint8_t i, j;

	for (i = nelements-1; (i >= 0) ; i--)
	{
		curr = array[i];
		if (curr == 0)
		{
			continue;
		}
		
		for (j = 31; j >= 0; j--)
		{ 
			if (curr & (1U << j)) //bitwise AND between curr and bitmask (non zero ouput means there is a 1 in position j)
			{
				return pos - ((7-i)*32); //if we skipped one, we need to take care of offset
			}
			pos--;
		}
	}
	return 0;
}