//--------------------------------------------------------------------------
#ifndef _INC_CRC16_H
#define _INC_CRC16_H
//--------------------------------------------------------------------------
extern const unsigned short KCrc16_0x1021[256];

static inline unsigned short Crc16Ccitt (unsigned short aCrc, unsigned char aData)
{
    return ((aCrc << 8) ^ KCrc16_0x1021[((aCrc >> 8) ^ aData) & 0xff]);
}


//--------------------------------------------------------------------------
#endif
