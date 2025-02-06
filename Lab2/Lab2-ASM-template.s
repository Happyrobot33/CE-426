	AREA MySub, CODE, READONLY	

	EXPORT findHighBitASM
		

findHighBitASM
	; where do you expect the subroutine input parameters to be passed in?
	; input parameters: R0: address of array, R1: number of elements in the array
	
	;Define the inputs
	#define arrPtr R0
	#define arrLen R1
	
	;Define the outputs
	#define returnLocation R0
	
	;Defined local vars
	#define arrIndex R2
	#define highestBit R3
	; According to the ABI for 32 bit arm processors, only r0-r3 are scratch registers, so everything else needs to be saved and restored once we are finished
	#define leadingZeros R4
	#define valueAtIndex R5

	; Define our stack saving list
	#define savedStack {valueAtIndex, leadingZeros}

	; Push all registers that are used
	PUSH savedStack

	;use R3 as a bit counter, initialize to 255 to assume the high bit of the array is the MSB
	LDR highestBit,=255
	MOV arrIndex, arrLen ;copy R1 to R2 to act as our active index into the array

loop1
	; Subtract 1 in order to get a proper index into the array
	SUB arrIndex, arrIndex, #1
	
	LDR valueAtIndex, [arrPtr, arrIndex, LSL #2]; ; Load the current int into valueAtIndex, using arrIndex as a index into the array. Multiply arrIndex by 4 due to addressing being in bytes instead of words
	CLZ leadingZeros, valueAtIndex ;Count the leading zeros into the leadingZeros register
	;Result will be 32 if no bits are set, or 0 if bit 31 is set
	;We can directly subtract this from highestBit which holds the bit position
	SUB highestBit, highestBit, leadingZeros
	
	;Check if we have 32, if so do loop again
	CMP leadingZeros, #32 ;check if the zero count is 32
	BEQ loop1 ;Jump to the start of the loop if leadingZeros - 32 is 0
	
	; Exit if we got to here as we found the value we want
	B done
	
done 
	
	MOV returnLocation, highestBit	; make sure that the final result is loaded in R0
	; restore saved registers
	POP savedStack
	
	BX LR			; return to the caller

	END