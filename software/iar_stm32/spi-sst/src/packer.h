//==========================================================================
// Buffer packing utils
//==========================================================================
#ifndef _INC_PACKER_H
#define _INC_PACKER_H

//==========================================================================
//==========================================================================
int PackFloat (unsigned char *aDest, float aValue);
int PackU32 (unsigned char *aDest, unsigned long aValue);
int PackU16 (unsigned char *aDest, unsigned short aValue);
int PackU8 (unsigned char *aDest, unsigned char aValue);
int PackString (unsigned char *aDest, const char *aSrc, int aLen);

//==========================================================================
// Buffer Unpack utils
//==========================================================================
float UnpackFloat (unsigned char *aSrc);
unsigned long UnpackU32 (unsigned char *aSrc);
unsigned short UnpackU16 (unsigned char *aSrc);
unsigned char UnpackU8 (unsigned char *aSrc);
void UnpackString (char *aDest, unsigned char *aSrc, int aLen);

//==========================================================================
//==========================================================================
#endif
