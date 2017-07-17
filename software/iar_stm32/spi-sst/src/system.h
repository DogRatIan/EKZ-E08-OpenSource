//==========================================================================
//==========================================================================
#ifndef _INC_SYSTEM_H
#define _INC_SYSTEM_H
//==========================================================================
//==========================================================================
void sys_exit_user (void);
unsigned short sys_version (void);
int sys_link_process (void);
int sys_link_read (unsigned char *aDest, int aLen);
int sys_link_write (const unsigned char *aSrc, int aLen);
void sys_led0_on (void);
void sys_led0_off (void);
unsigned long sys_get_tick (void);

//==========================================================================
//==========================================================================
#endif
