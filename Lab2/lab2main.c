//***********************************************************************************************************
//  Author: Girma Tewolde
//	Last modified: 1-25-2023 @5:52 P.M.
//  Purpose: Framework for CE-426 Lab 2
//
//  Use the array data defined in the global memory space and the main program in this file for testing
//  your two versions, i.e. assembly and C, of the findHighBit function.
//  Test each of your two implementations with the given array data and make sure your answers match 
//  with the ones provided in the comments for each array.
//  You will also compare the two implementations on criteria such as execution speed and size of 
//  memory used by the corresponding functions.
//***********************************************************************************************************

#include <stm32f10x.h>

//last index is the most significant
//use CLZ (count leading zeros) to find the specific bit in the int
uint32_t array0[] = {0x00000001,0x00000020,0x00000400,0x00008000,0x00440000,0x02200000,0x12000000,0x80000000}; // answer = 255; 
uint32_t array1[] = {0x00000001,0x00000020,0x00000400,0x00008000,0x00440000,0x02200000,0x12000000,0x00000000}; // answer = 220; 
uint32_t array2[] = {0x00000001,0x00000020,0x00000400,0x00008000,0x00440000,0x02200000,0x00000000,0x00000000}; // answer = 185; 
uint32_t array3[] = {0x00000001,0x00000020,0x00000400,0x00008000,0x00440000,0x00000000,0x00000000,0x00000000}; // answer = 150; 
uint32_t array4[] = {0x00000001,0x00000020,0x00000400,0x00008000,0x00000000,0x00000000,0x00000000,0x00000000}; // answer = 111; 
uint32_t array5[] = {0x00000001,0x00000020,0x00000400,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}; // answer = 74; 
uint32_t array6[] = {0x00000001,0x00000020,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}; // answer = 37; 
uint32_t array7[] = {0x00000001,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000}; // answer = 0; 

uint32_t *arrays[] = {array0, array1, array2, array3, array4, array5, array6, array7};
uint32_t narrays = sizeof(arrays)/sizeof(uint32_t*);
uint32_t nelements = sizeof(array0)/sizeof(uint32_t);

uint32_t findHighBitASM(uint32_t*, uint32_t);
uint32_t findHighBitC(uint32_t*, uint32_t);

int main(void){
	uint8_t i;
	volatile uint32_t positionASM;
	volatile uint32_t positionC;
	
	//iterate through each of the test arrays
	for (i = 0; i < narrays; i++)
	{
		positionASM = findHighBitASM(arrays[i], nelements);
		positionC = findHighBitC(arrays[i], nelements);
	}
	
	while(1){} // endless loop to keep micro from crashing
}
