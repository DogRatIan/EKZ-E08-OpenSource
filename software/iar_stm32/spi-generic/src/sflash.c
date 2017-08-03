//==========================================================================
// Serial FLASH
//==========================================================================
#include "mcu.h"
#include "spi.h"
#include "sflash.h"

//==========================================================================
//==========================================================================
static int g4ByteAddr;

//==========================================================================
// Initialize
//==========================================================================
void SflashInit (void)
{
    g4ByteAddr = 0;
}

//==========================================================================
// Get Manufacturer ID and Device ID
//==========================================================================
unsigned long SflashReadId (void)
{
    unsigned long id;

    // Escape
    SpiSetCs (0);
    SpiByte (0xff);
    SpiSetCs (1);

    // Try 0x9F JEDEC ID first
    SpiSetCs (0);
    SpiByte (0x9f);
    id = ((unsigned long)SpiByte (0xff) << 24);
    id |= ((unsigned long)SpiByte (0xff) << 16);
    id |= ((unsigned long)SpiByte (0xff) << 8);
    id |= 0xff;
    SpiSetCs (1);

    if ((id == 0x000000ff) || (id == 0xffffffff))
    {
        // Escape
        SpiSetCs (0);
        SpiByte (0xff);
        SpiSetCs (1);

        // Fail, try 0x90
        SpiSetCs (0);
        SpiByte (0x90);
        SpiByte (0x00);
        SpiByte (0x00);
        SpiByte (0x00);
        id = ((unsigned long)SpiByte (0xff) << 24);
        id |= ((unsigned long)SpiByte (0xff) << 16);
        id |= 0xffff;
        SpiSetCs (1);
    }

    return id;
}

//==========================================================================
// Get Status Register 1
//      D7: Status Register protected
//      D6: Sector protected
//      D5: Top/Bottom protected
//  D4..D2: Block protected bits
//      D1: Write Enable Latch
//      D0: Busy
//==========================================================================
unsigned char SflashReadStatus1 (void)
{
    unsigned char ret;

    SpiSetCs (0);
    SpiByte (0x05);
    ret = SpiByte (0xFF);
    SpiSetCs (1);

    return ret;
}

//==========================================================================
// Get Status Register 2
//      D7: Suspend Status
//  D6..D2: Reserved
//      D1: Quad Enable
//      D0: Status Register protected
//==========================================================================
unsigned char SflashReadStatus2 (void)
{
    unsigned char ret;

    SpiSetCs (0);
    SpiByte (0x35);
    ret = SpiByte (0xFF);
    SpiSetCs (1);

    return ret;
}

//==========================================================================
// Write Status Register 1
//==========================================================================
void SflashWriteStatus1 (unsigned char aStatus)
{
    SpiSetCs (0);
    SpiByte (0x01);
    SpiByte (aStatus);
    SpiSetCs (1);
}


//==========================================================================
// Write Status Register
//  MSB: Status Register 2
//  LSB: Status Register 1
//==========================================================================
void SflashWriteStatus (unsigned short aStatus)
{
    SpiSetCs (0);
    SpiByte (0x01);
    SpiByte (aStatus >> 0);
    SpiByte (aStatus >> 8);
    SpiSetCs (1);
}

//==========================================================================
// Write Enable
//==========================================================================
void SflashWriteEnable (void)
{
    SpiSetCs (0);
    SpiByte (0x06);
    SpiSetCs (1);

}

//==========================================================================
// Write Disable
//==========================================================================
void SflashWriteDisable (void)
{
    SpiSetCs (0);
    SpiByte (0x04);
    SpiSetCs (1);

}

//==========================================================================
// Chip Erase
//==========================================================================
void SflashChipErase (void)
{
    SpiSetCs (0);
    SpiByte (0xc7);
//    SpiByte (0x60);
    SpiSetCs (1);

}

//==========================================================================
// Sector Erase
//==========================================================================
void SflashSectorErase (unsigned long aAddr)
{
    SpiSetCs (0);
    SpiByte (0x20);
    if (g4ByteAddr)
    {
        SpiByte (aAddr >> 24);
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    else
    {
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    SpiSetCs (1);

}

//==========================================================================
// Write a Page
//==========================================================================
void SflashWritePage (unsigned long aAddr, const unsigned char *aSrc)
{
    int i;

    // Align address to page
    aAddr &= 0xffff00;

    //
    SpiSetCs (0);
    SpiByte (0x02);
    if (g4ByteAddr)
    {
        SpiByte (aAddr >> 24);
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    else
    {
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    for (i = 256; i; i--)
    {
        SpiByte (*aSrc);
        aSrc ++;
    }
    SpiSetCs (1);
}

//==========================================================================
// Read a Page
//==========================================================================
void SflashReadPage (unsigned long aAddr, unsigned char *aDest)
{
    int i;

    // Align address to page
    aAddr &= 0xffff00;

    //
    SpiSetCs (0);
    SpiByte (0x03);
    if (g4ByteAddr)
    {
        SpiByte (aAddr >> 24);
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    else
    {
        SpiByte (aAddr >> 16);
        SpiByte (aAddr >> 8);
        SpiByte (aAddr >> 0);
    }
    for (i = 256; i; i--)
    {
        *aDest = SpiByte (0xff);
        aDest ++;
    }
    SpiSetCs (1);
}

////==========================================================================
//// Set a Addr
////==========================================================================
//void SflashWriteAddr (unsigned short SFCMD, unsigned long aAddr)
//{
//    //
//    SpiSetCs (0);
//    SpiByte (SFCMD);
//    SpiByte (aAddr >> 16);
//    SpiByte (aAddr >> 8);
//    SpiByte (aAddr >> 0);
//
//}

//==========================================================================
// Enter/Exit 4-Byte Address Mode
//==========================================================================
void Sflash4ByteAddrMode (int aEnable)
{
    SpiSetCs (0);
    if (aEnable)
        SpiByte (0xb7);
    else
        SpiByte (0xe9);
    SpiSetCs (1);
}


//==========================================================================
// Reset
//==========================================================================
void SflashReset (void)
{
    SpiSetCs (0);
    SpiByte (0xff);
    SpiSetCs (1);
}
