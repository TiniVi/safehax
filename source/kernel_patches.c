#include "kernel_patches.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "global_backdoor.h"


// Shamelessly copied from
// https://github.com/Steveice10/memchunkhax2/blob/master/source/memchunkhax2.c#L16

#define OLDNEW(x)                    (g_is_new3ds ? x ## _NEW : x ## _OLD)

#define CURRENT_KTHREAD              (*((u8**)0xFFFF9000))
#define CURRENT_KPROCESS             (*((u8**)0xFFFF9004))

#define SVC_ACL_SIZE                 (0x10)

#define KPROCESS_ACL_START_OLD       (0x88)
#define KPROCESS_ACL_START_NEW       (0x90)

#define KPROCESS_PID_OFFSET_OLD      (0xB4)
#define KPROCESS_PID_OFFSET_NEW      (0xBC)

#define KTHREAD_THREADPAGEPTR_OFFSET (0x8C)
#define KSVCTHREADAREA_BEGIN_OFFSET  (0xC8)


static bool g_is_new3ds = false;
static u32 g_original_pid = 0;

static void K_PatchPID(void)
{
    u8 *proc = CURRENT_KPROCESS;
    u32 *pidPtr = (u32*)(proc + OLDNEW(KPROCESS_PID_OFFSET));

    g_original_pid = *pidPtr;

    // We're now PID zero, all we have to do is reinitialize the service manager in user-mode.
    *pidPtr = 0;
}

static void K_RestorePID(void)
{
    u8 *proc = CURRENT_KPROCESS;
    u32 *pidPtr = (u32*)(proc + OLDNEW(KPROCESS_PID_OFFSET));

    // Restore the original PID
    *pidPtr = g_original_pid;
}

static void K_PatchACL(void)
{
    // Patch the process first (for newly created threads).
    u8 *proc = CURRENT_KPROCESS;
    u8 *procacl = proc + OLDNEW(KPROCESS_ACL_START);
    memset(procacl, 0xFF, SVC_ACL_SIZE);

    // Now patch the current thread.
    u8 *thread = CURRENT_KTHREAD;
    u8 *thread_pageend = *(u8**)(thread + KTHREAD_THREADPAGEPTR_OFFSET);
    u8 *thread_page = thread_pageend - KSVCTHREADAREA_BEGIN_OFFSET;
    memset(thread_page, 0xFF, SVC_ACL_SIZE);
}


void initsrv_allservices(void)
{
    APT_CheckNew3DS(&g_is_new3ds);

    printf("Patching PID\n");
    svc_30(K_PatchPID);

    printf("Reiniting srv:\n");
    srvExit();
    srvInit();

    printf("Restoring PID\n");
    svc_30(K_RestorePID);
}

void patch_svcaccesstable(void)
{
    APT_CheckNew3DS(&g_is_new3ds);

    printf("Patching SVC access table\n");
    svc_30(K_PatchACL);
}
