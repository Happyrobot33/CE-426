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
	
	;PUSH ...		; do you need to preserve registers that you may end up using ?

	;use R3 as a bit counter, initialize to 255 to assume the high bit of the array is the MSB
	LDR R3,=255
	MOV R2, R1 ;copy R1 to R2 to act as our active index into the array

loop1
	; Subtract 1 in order to get a proper index into the array
	SUB R2, R2, #1
	
	LDR R5, [R0, R2, LSL #2]; ; Load the current int into R5, using R1 as a index into the array. Multiply R2 by 4 due to addressing being in bytes instead of 32 bit ranges
	CLZ R4, R5 ;Count the leading zeros into the R4 register.
	;Result will be 32 if no bits are set, or 0 if bit 31 is set
	;We can directly subtract this from R3 which holds the bit position R3 = R3 - R4
	SUB R3, R3, R4
	
	;Check if we have 32, if so do loop again
	CMP R4, #32 ;check if the zero count is 32
	BEQ loop1 ;Jump to the start of the loopif R4 - 32 is 0
	
	; Exit if we got to here as we found the value we want
	B done
	
done 
	
	MOV R0, R3 ;		; make sure that the final result is loaded in R0
	;POP ...			; restore saved registers
	BX LR			; return to the caller

	END