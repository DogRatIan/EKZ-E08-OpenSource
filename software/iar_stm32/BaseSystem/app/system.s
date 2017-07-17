//=============================================================================
//=============================================================================
    MODULE  ?system

//=============================================================================
//=============================================================================
    PUBLIC  FuncTable
    PUBLIC  sys_enter_user
    PUBLIC  sys_exit_user

    EXTERN  sys_link_process
    EXTERN  sys_link_read
    EXTERN  sys_link_write
    EXTERN  GetTick
    EXTERN  Led0On
    EXTERN  Led0Off


//=============================================================================
//=============================================================================
    RSEG    .functable:CODE:NOROOT(2)
    DATA
FuncTable:
    DC32    0x24681357
    DC32    sys_version
    DC32    sys_exit_user
    DC32    sys_link_process
    DC32    sys_link_read
    DC32    sys_link_write
    DC32    Led0On
    DC32    Led0Off
    DC32    GetTick

//=============================================================================
//=============================================================================
    RSEG    .text:CODE:REORDER(1)
    THUMB
sys_enter_user:
    cpsid i                 // Disable interrupt

    push {r0-r12, lr}       // Save registers
    movw r0, #0x1f80        // Save SP
    movt r0, #0x2000
    str sp, [r0]

    movw r0, #0x2000        // Load User Program SP
    movt r0, #0x2000
    ldr sp, [r0]

    cpsie i                 // Enable interrupt

    movw r0, #0x2004        // Jump to User Program
    movt r0, #0x2000
    ldr r0, [r0]
    bx  r0

sys_exit_user:
    cpsid i                 // Disable interrupt
    movw r0, #0x1f80        // Restore SP
    movt r0, #0x2000
    ldr sp, [r0]
    
    cpsie i                 // Enable interrupt

    pop {r0-r12,pc}         // Restore registers

sys_version:
    movw r0, #0x0100
    bx  lr

//=============================================================================
//=============================================================================
    END
