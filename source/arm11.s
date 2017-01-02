.arm
.text

.global	arm11_start
arm11_start:
	@ clear the arm11 entrypoint
	LDR             R0, =0x1FFFFFFC
	MOV             R1, #0
	STR             R1, [R0]
	
	@ send pxi cmd 0x44846
	LDR             R1, =0x10163008
	LDR             R2, =0x44846
	STR             R2, [R1]
	
	@ wait for ARM9 to set the ARM11 entrypoint
wait_arm11_loop:
	LDR	            R1, [R0]
	CMP             R1, #0
	BEQ             wait_arm11_loop
	
	@ execute arbitrary ARM11 payload while the newly loaded ARM9 is running
	LDR             R1, =0x23FFF000
	BX R1
.pool

.global arm11_end
arm11_end:
