#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "global_backdoor.h"
#include "kernel_patches.h"

#define FCRAM(x)   (void *)((kver < SYSTEM_VERSION(2, 44, 6)) ? (0xF0000000 + x) : (0xE0000000 + x)) //0x20000000
#define AXIWRAM(x) (void *)((kver < SYSTEM_VERSION(2, 44, 6)) ? (0xEFF00000 + x) : (0xDFF00000 + x)) //0x1FF00000
#define KMEMORY    ((u32 *)AXIWRAM(0xF4000))                                                         //0x1FFF4000

//if condition 'x' is true, print string 'y' and exit
#define PANIC(x,y) if (x){\
                       error = y;\
                       goto exit;\
                   }
#define DEBUG(x)   if (debug) printf(" %s\n", x);

static Result pm_res = -1;
static s32 backdoor_res = -1;
static char *error = "FAILED TO LAUNCH SAFE_MODE ARM9!";

static void *payload_buf = NULL;
static u32 payload_size = 0;

static bool debug = false;
static u32 kver = 0;

extern void gfxSetFramebufferInfo(gfxScreen_t screen, u8 id);

s32 patch_arm11_codeflow(void){
	__asm__ volatile ( "CPSID AIF\n" "CLREX" );
	
	memcpy(FCRAM(0x3F00000), payload_buf, payload_size);
	memcpy(FCRAM(0x3FFF000), payload_buf + 0xFF000, 0xE0C);
	
	for (unsigned int i = 0; i < 0x2000/4; i++){
		if (KMEMORY[i] == 0xE12FFF14 && KMEMORY[i+2] == 0xE3A01000){ //hook arm11 launch
			KMEMORY[i+3] = 0xE51FF004; //LDR PC, [PC,#-4]
			KMEMORY[i+4] = 0x23FFF000;
			backdoor_res = 0;
			break;
		}
	}
	
	__asm__ volatile ( //flush & invalidate the caches
		"MOV R0, #0\n"
		"MCR P15, 0, R0, C7, C10, 0\n"
		"MCR P15, 0, R0, C7, C5, 0"
	);
	
	return backdoor_res;
}

u32 FileRead(void *buffer, const char *filename, u32 maxsize){ //lol
	u32 size = 0;
	FILE * handle = fopen(filename, "rb");
	if (handle){
		fseek(handle, 0, SEEK_END);
		size = ftell(handle);
		rewind(handle);
		
		if (size && (size <= maxsize)) fread(buffer, 1, size, handle);
		else size = 0;
		fclose(handle);
	}
	return size;
}

int main(int argc, char **argv){
	gfxInitDefault();
	fsInit();
	aptInit();
	sdmcInit();
	romfsInit();
	
	if (checkSvcGlobalBackdoor()){
		initsrv_allservices();
		patch_svcaccesstable();
    }
	
	PANIC(pmInit(), "PM INIT FAILED!");
	
	hidScanInput();
	if (hidKeysDown() & KEY_B){ //Hold B to enable prints
		consoleInit(GFX_TOP, NULL);
		printf("\n\x1b[37;1m");
		debug = true;
	}
	
	/* Map the Payloads */
	
	DEBUG("Allocating memory...");
	payload_buf = memalign(0x1000, 0x100000);
	PANIC(!payload_buf, "FAILED TO ALLOCATE MEMORY!");
	
	DEBUG("Reading ARM9 payload...");
	payload_size = FileRead(payload_buf, "sdmc:/safehaxpayload.bin", 0xFF000); //MINOR: Large payloads seem to crash when being copied to 0x23F00000
	if (!payload_size) payload_size = FileRead(payload_buf, "sdmc:/arm9.bin", 0xFF000);
	if (!payload_size) payload_size = FileRead(payload_buf, "sdmc:/arm9loaderhax.bin", 0xFF000);
	PANIC(!payload_size, "FAILED TO READ THE ARM9 PAYLOAD!");
	
	DEBUG("Reading ARM11 payload...");
	PANIC(!FileRead(payload_buf + 0xFF000, "romfs:/arm11.bin", 0xE00), "FAILED TO READ THE ARM11 PAYLOAD!"); //If this fails, it's either an old hbl, or I'm retarded
	
	/* Setup Framebuffers */ //https://github.com/mid-kid/CakeBrah/blob/master/source/brahma.c#L364
	
	DEBUG("Setting framebuffers...");
	*((u32*)(payload_buf + 0xFFE00)) = (u32)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL) + 0xC000000;
	*((u32*)(payload_buf + 0xFFE04)) = (u32)gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL) + 0xC000000;
	*((u32*)(payload_buf + 0xFFE08)) = (u32)gfxGetFramebuffer(GFX_BOTTOM, 0, NULL, NULL) + 0xC000000;
	gfxSwapBuffers();
	
	/* Patch ARM11 */
	
	kver = osGetKernelVersion();
	
	DEBUG("Patching ARM11...");
	svcBackdoor(patch_arm11_codeflow);
	PANIC(backdoor_res, "FAILED TO PATCH THE KERNEL!");
	
	/* Launch SAFE_MODE ARM9 */
	
	DEBUG("Launching SAFE_MODE ARM9...");
	pm_res = PM_LaunchFIRMSetParams(3, 0, NULL);
	
exit:
	if (pm_res){
		if (!debug) consoleInit(GFX_TOP, NULL);
		printf("\n\x1b[31;1m [!] %s\n", error);
		printf("\n\x1b[37;1m Press [START] to exit.");
		
		while (aptMainLoop()){
			hidScanInput();
			
			if (hidKeysDown() & KEY_START) break;
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			gspWaitForVBlank();
		}
	} else if (debug){ //fix framebuffer on exit
		consoleClear();
		gfxSetScreenFormat(GFX_TOP, GSP_BGR8_OES);
		gfxSetFramebufferInfo(GFX_TOP, 0);
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
	
	pmExit();
	romfsExit();
	sdmcExit();
	aptExit();
	fsExit();
	gfxExit();
	return 0;
}
