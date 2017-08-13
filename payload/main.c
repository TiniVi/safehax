#define PXI_SYNC11  ((volatile unsigned char *)0x10163000)
#define PXI_CNT11   *((volatile unsigned int *)0x10163004)
#define PXI_SEND11  *((volatile unsigned int *)0x10163008)
#define PXI_RECV11  *((volatile unsigned int *)0x1016300C)

#define ARM9_TEST   *((volatile unsigned int *)0x1FF80000)
#define ARM9SYNC    ((volatile unsigned char *)0x1FFFFFF0)
#define ARM11Entry  *((volatile unsigned int *)0x1FFFFFF8)
#define ARM9Entry   *((volatile unsigned int *)0x2400000C)

#define FW_TID_HIGH 0x00040138
#define FW_TID_LOW  ((*((volatile unsigned int *)0x10140FFC) == 7) ? 0x20000003 : 0x00000003)
#define PXI_FULL    (PXI_CNT11 & 2)
#define PXI_EMPTY   (PXI_CNT11 & 0x100)

#define LCDCFG_FILL *((volatile unsigned int *)0x10202204)
#define DEBUG_FLAG  *((volatile _Bool *)0x23FFFE10)
#define DEBUG(c)    if (DEBUG_FLAG) LCDCFG_FILL = 0x01000000 | (c & 0xFFFFFF);

void pxi_send(unsigned int cmd){
	while (PXI_FULL);
	PXI_SEND11 = cmd;
}

unsigned int pxi_recv(void){
	while (PXI_EMPTY);
	return PXI_RECV11;
}

void _start(void){
	DEBUG(0x0000FF); //red
	
	for (volatile int i = 0; i < 2; i++){ //The global SAFE_MODE flag is cleared now, let's launch SAFE_MODE!
		/* Init */
		pxi_send(0x44846); //Signal ARM9 to complete the initial FIRMLaunches
		
		/* KSync */
		ARM9SYNC[0] = 1; //SAFE_MODE Kernel9 @ 0x0801B148 & SAFE_MODE Kernel11 @ 0x1FFDB498
		while (ARM9SYNC[0] != 2);
		ARM9SYNC[0] = 1;
		while (ARM9SYNC[0] != 2);
		
		DEBUG(0x007FFF); //orange
		
		if (ARM9SYNC[1] == 3){
			ARM9SYNC[0] = 1;
			while (ARM9SYNC[0] != 2);
			ARM9SYNC[0] = 3;
		}
		
		while (!PXI_FULL) pxi_send(0); //SAFE_MODE Process9 @ 0x0806C594 & SAFE_MODE pxi @ 0x101388
		PXI_SYNC11[1] = 1;
		while (PXI_SYNC11[0] != 1);
		
		DEBUG(0x00FFFF); //yellow
		
		while (!PXI_EMPTY) pxi_recv();
		PXI_SYNC11[1] = 2;
		while (PXI_SYNC11[0] != 2);
		
		DEBUG(0x00FF00); //green
		
		/* FIRMLaunch */
		pxi_send(0); //pxi:mc //https://github.com/patois/Brahma/blob/master/source/arm11.s#L11 & SAFE_MODE pxi @ 0x100618
		PXI_SYNC11[3] |= 0x40;
		pxi_send(0x10000); //pxi shutdown
		
		DEBUG(0xFFFF00); //cyan
		
		do pxi_send(0x44836); //SAFE_MODE Process9 @ 0x08086788 & SAFE_MODE Kernel11 @ 0xFFF620C0
		while (PXI_EMPTY || (pxi_recv() != 0x964536));
		pxi_send(0x44837);
		
		pxi_send(FW_TID_HIGH);
		pxi_send(FW_TID_LOW);
		
		DEBUG(0xFF0000); //blue
	}
	
	DEBUG(0xFF007F); //purple
	
	/* FIRMLaunchHax */
	ARM11Entry = 0;
	ARM9_TEST = 0xDEADC0DE;
	pxi_send(0x44846);
	
	volatile unsigned int reg = 0;
	while (ARM9_TEST == 0xDEADC0DE) //wait for arm9 to start writing a new arm11 binary
		__asm__ volatile ("MCR P15, 0, %0, C7, C6, 0" :: "r"(reg)); //invalidate dcache
	
	ARM9Entry = 0x23F00000;
	LCDCFG_FILL = 0;
	while (!ARM11Entry);
	((void (*)())ARM11Entry)();
}
