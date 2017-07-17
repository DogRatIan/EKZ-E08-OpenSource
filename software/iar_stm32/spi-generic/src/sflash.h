//==========================================================================
//==========================================================================
#ifndef _INC_SFLASH_H
#define _INC_SFLASH_H

//==========================================================================
//==========================================================================
void SflashInit(void);
unsigned long SflashReadId (void);
unsigned char SflashReadStatus1 (void);
unsigned char SflashReadStatus2 (void);
void SflashWriteStatus1 (unsigned char aStatus);
void SflashWriteStatus (unsigned short aStatus);
void SflashWriteEnable (void);
void SflashWriteDisable ();
void SflashChipErase ();
void SflashSectorErase (unsigned long aAddr);
void SflashWritePage (unsigned long aAddr, const unsigned char *aSrc);
void SflashReadPage (unsigned long aAddr, unsigned char *aDest);
void SflashReset (void);

//==========================================================================
//==========================================================================
#endif
