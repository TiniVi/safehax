.arm
.section .text.start

CPSID AIF
MOV SP, #0x27800000

MOV R0, #0
MCR P15, 0, R0, C7, C10, 0
MCR P15, 0, R0, C7, C5, 0

BL _start
B .
