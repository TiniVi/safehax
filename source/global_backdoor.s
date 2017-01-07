.global svc_30
.type   svc_30, %function
svc_30:
    push {r0, r1, r2}
    mov r3, sp
    adr r0, svc_30_callback
    svc 0x30
    add sp, sp, #8
    ldr r0, [sp], #4
    bx lr
    svc_30_callback:
        cpsid aif
        ldr r2, [r3], #4
        ldmfd r3!, {r0, r1}
        push {r3, lr}
        blx r2
        pop {r3, lr}
        str r0, [r3, #-4]!
        mov r0, #0
        bx lr


.global svcGlobalBackdoor
.type   svcGlobalBackdoor, %function
svcGlobalBackdoor:
    svc 0x30
    bx lr


.global checkSvcGlobalBackdoor
.type   checkSvcGlobalBackdoor, %function
checkSvcGlobalBackdoor:
    adr r0, checkSvcGlobalBackdoor_callback
    mov r3, #0
    svc 0x30
    mov r0, r3
    bx lr
    checkSvcGlobalBackdoor_callback:
        cpsid aif
        mov r3, #1
        bx lr
