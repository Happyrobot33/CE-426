;.............................................................................
; findHighBitASM - assembly implementation of the find high bit function
;
; ... provide complete documentation
; ... description - what the subroutine does
; ... what are the input and output parameters and how/where they are passed
; ... briefly describe how the algorithm works
; ... 
; ... write names of the group members 
; ... include last modification date
;.............................................................................

	AREA MySub, CODE, READONLY	

	EXPORT findHighBitASM
		

findHighBitASM
	; where do you expect the subroutine input parameters to be passed in?
	; input parameters: R0: address of array, R1: number of elements in the array
	;Define the inputs
	#define arrPtr R0
	#define arrLen R1
	#define returnLocation R0

	;PUSH ...		; do you need to preserve registers that you may end up using ?

	;use R3 as a bit counter, initialize to 255 to assume the high bit of the array is the MSB
	#define highestBit R3
	LDR highestBit,=255
	#define arrIndex R2
	MOV arrIndex, R1 ;copy R1 to R2 to act as our active index into the array

loop1
	; Subtract 1 in order to get a proper index into the array
	SUB arrIndex, arrIndex, #1
	
	#define valueAtIndex R5
	LDR valueAtIndex, [arrPtr, arrIndex, LSL #2]; ; Load the current int into valueAtIndex, using arrIndex as a index into the array. Multiply arrIndex by 4 due to addressing being in bytes instead of words
	#define leadingZeros R4
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
	;POP ...			; restore saved registers
	BX LR			; return to the caller

	END