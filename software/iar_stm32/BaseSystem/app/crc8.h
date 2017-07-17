//--------------------------------------------------------------------------
#ifndef _INC_CRC8_H
#define _INC_CRC8_H
//--------------------------------------------------------------------------
extern const unsigned char KCrc8_0x07[256];

static inline unsigned char Crc8 (unsigned char aCrc, unsigned char aData)
{
    return KCrc8_0x07[aCrc ^ aData];
}

//--------------------------------------------------------------------------
#endif
