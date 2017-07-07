.arm
.section .text.start

CPSID AIF
MOV SP, #0x27800000

MOV R0, #0
MCR P15, 0, R0, C7, C10, 0
MCR P15, 0, R0, C7, C5, 0

BL _start
B .

.global _firmldr
.type _firmldr, %function
_firmldr:
	MSR CPSR_c, #0xD3          @ Disable IRQs, context switch to SVC mode
	MOV SP, #0x27000000
	
	MRC P15, 0, R0, C1, C0, 0  @ Disable MPU, ICache, and DCache
	BIC R0, R0, #0x1000
	BIC R0, R0, #5
	MCR P15, 0, R0, C1, C0, 0
	
	LDR R0, =0x33333333        @ Set all 8 MPU regions as Instruction & Data Regions for Kernel & User
	MCR P15, 0, R0, C5, C0, 2
	MCR P15, 0, R0, C5, C0, 3
	
	LDR R0, =0xFFF00027        @ Data TCM & BOOT9
	MCR P15, 0, R0, C6, C0, 0
	LDR R0, =0x00000035        @ Instruction TCM
	MCR P15, 0, R0, C6, C1, 0
	LDR R0, =0x08000029        @ ARM9 Private Memory Region
	MCR P15, 0, R0, C6, C2, 0
	LDR R0, =0x10000035        @ IO Registers
	MCR P15, 0, R0, C6, C3, 0
	LDR R0, =0x1800002D        @ 6MB VRAM
	MCR P15, 0, R0, C6, C4, 0
	LDR R0, =0x1FF00025        @ DSP Memory
	MCR P15, 0, R0, C6, C5, 0
	LDR R0, =0x1FF80025        @ AXIWRAM
	MCR P15, 0, R0, C6, C6, 0
	LDR R0, =0x20000037        @ FCRAM
	MCR P15, 0, R0, C6, C7, 0
	
	MOV R0, #0xF4              @ Enable instruction/data caching and write buffering on all RAM
	MCR P15, 0, R0, C2, C0, 0
	MCR P15, 0, R0, C2, C0, 1
	MCR P15, 0, R0, C3, C0, 0
	
	MOV R0, #0                 @ Invalidate caches, flush write buffer
	MCR P15, 0, R0, C7, C5, 0
	MCR P15, 0, R0, C7, C6, 0
	MCR P15, 0, R0, C7, C10, 4
	
	MRC P15, 0, R0, C1, C0, 0  @ Enable MPU, ITCM, DTCM, ICache, and DCache
	ORR R0, R0, #0x51000
	ORR R0, R0, #5
	MCR P15, 0, R0, C1, C0, 0
	
	BL firmldr
	B .
.pool
