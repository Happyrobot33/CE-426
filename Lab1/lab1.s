;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CE426 - Real-Time Embedded Systems
; Instructor: Professor Tewolde
; Author: Girma Tewolde
; Lab1: Keil uVision and ARM Assembly
;
; Description: This sample code is provided to learn about the assembly programming
; process on an ARM Cortex M3 Microcontroller. Follow the instructions in the lab
; assignment document to create the project and execute the program using a debug tool.
;
; Harware: none (Program will be run in a simulation environment)
;
; Program input/output: the input data is provided in memory and the output is also
; stored back in memory.
;
; Last modified: 01/18/2018 @ 9:53 a.m.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	AREA MyData, CODE
	
array1
	DCD 0x87654321, 0x23456789, 0x3456789A
array2
	DCD 0xAABBCCDD, 0x7FCCDDEE, 0xCCDDEEFF
	
	AREA MyAnswers, DATA
array3
	SPACE 12


	AREA MyCode, CODE, READONLY
	EXPORT 	SystemInit
	EXPORT	__main
	
SystemInit
	BX	LR

__main
	ENTRY
	LDR R0, =array1
	LDR R1, =array2
	LDR R2, =array3
	
	LDR R3, [R0], #4
	LDR R4, [R1], #4
	ADDS R4, R3, R4
	STR R4, [R2], #4
	
	LDR R3, [R0], #4
	LDR R4, [R1], #4
	ADDS R4, R3, R4
	STR R4, [R2], #4	
	
	LDR R3, [R0]
	LDR R4, [R1]
	ADDS R4, R3, R4
	STR R4, [R2]	
	
	B	.
	END
