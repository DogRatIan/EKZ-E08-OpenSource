//==========================================================================
//==========================================================================
#include "system.h"

typedef void (* func_void_0)(void);
typedef int (* func_int_0)(void);
typedef int (* func_int_2)(unsigned long a1, unsigned long a2);
typedef unsigned short (* func_u16_0)(void);
typedef unsigned long (* func_u32_0)(void);


//==========================================================================
// Exit and back to system
//==========================================================================
void sys_exit_user (void)
{
    register func_void_0 sys_call_entry;
    sys_call_entry = * ((func_void_0 *)0x8000108);
    sys_call_entry ();
}

//==========================================================================
// Get version of BaseSystem
//==========================================================================
unsigned short sys_version (void)
{
    register func_u16_0 sys_call_entry;
    sys_call_entry = * ((func_u16_0 *)0x8000104);
    return sys_call_entry ();
}

//==========================================================================
// Link layer process
//  Return -ve value when there is failure, 
//  User program should exit and back to system when it happen.
//==========================================================================
int sys_link_process (void)
{
    register func_int_0 sys_call_entry;
    sys_call_entry = * ((func_int_0 *)0x800010c);
    return sys_call_entry ();
}

//==========================================================================
// Get received command
//==========================================================================
int sys_link_read (unsigned char *aDest, int aLen)
{
    register func_int_2 sys_call_entry;
    sys_call_entry = * ((func_int_2 *)0x8000110);
    return sys_call_entry ((unsigned long)aDest, (unsigned long)aLen);
}

//==========================================================================
// Send out response
//==========================================================================
int sys_link_write (const unsigned char *aSrc, int aLen)
{
    register func_int_2 sys_call_entry;
    sys_call_entry = * ((func_int_2 *)0x8000114);
    return sys_call_entry ((unsigned long)aSrc, (unsigned long)aLen);
}

//==========================================================================
// Turn on LED0
//==========================================================================
void sys_led0_on (void)
{
    register func_void_0 sys_call_entry;
    sys_call_entry = * ((func_void_0 *)0x8000118);
    sys_call_entry ();
}

//==========================================================================
// Turn off LED0
//==========================================================================
void sys_led0_off (void)
{
    register func_void_0 sys_call_entry;
    sys_call_entry = * ((func_void_0 *)0x800011c);
    sys_call_entry ();
}

//==========================================================================
// Get System Tick counter
//==========================================================================
unsigned long sys_get_tick (void)
{
    register func_u32_0 sys_call_entry;
    sys_call_entry = * ((func_u32_0 *)0x8000120);
    return sys_call_entry ();
}
