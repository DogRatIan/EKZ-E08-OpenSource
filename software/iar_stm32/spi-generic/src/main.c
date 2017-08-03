//==========================================================================
//==========================================================================
#include <stdio.h>

#include "define.h"
#include "mcu.h"
#include "system.h"
#include "tick.h"
#include "spi.h"
#include "sflash.h"
#include "packer.h"

//==========================================================================
// Defines
//==========================================================================
#define CMD_GET_INFO            0x00
#define CMD_ECHO                0xff
#define RESP_ERROR              0x80
#define CMD_EXIT                0x01
#define CMD_CHIP_INFO           0x02
#define CMD_CHIP_ERASE          0x03
#define CMD_READ_DATA           0x04
#define CMD_WRITE_DATA          0x05
#define CMD_READ_REG            0x06
#define CMD_WRITE_REG           0x07
#define CMD_BLANK_CHECK         0x08
#define CMD_ENTER_4BYTE_ADDR    0x09
#define CMD_EXIT_4BYTE_ADDR     0x0A

#define TIMEOUT_DEFAULT     1000
#define TIMEOUT_ERASE       60000

//==========================================================================
// Variables
//==========================================================================
static unsigned char gBuf[1280];
static int gReady;
static unsigned long gTickLed;

//==========================================================================
// Constants
//==========================================================================
static const char *KName="SPI_GENERIC";
static const unsigned short KVersion = 0x0100;

//==========================================================================
// Wait ready
//==========================================================================
static inline int WaitSflashReady (unsigned long aTimeout)
{
    unsigned char status;
    unsigned long tick;
    unsigned long l;

    if (gReady)
        return 0;;

    tick = GetTick ();
    do
    {
        status = SflashReadStatus1 ();
        if ((status & 0x01) == 0)
        {
            gReady = true;
            return 0;
        }

        gTickLed = GetTick ();
        l = gTickLed & 0x7f;
        if (l == 0)
            sys_led0_on ();
        else if (l == 0x40)
            sys_led0_off ();

    } while (TickElapsed (tick) < aTimeout);

    return -1;
}

//==========================================================================
// IO Init
//==========================================================================
void IoInit(void)
{
    // Set GPIO Mode for SPI
    GPIOA_BSRR = 0x00;
    GPIOA_CRL &= 0x0000f0f0;
    GPIOA_CRL |= 0xaaa20202;
    GPIOA_CRH &= 0xfffffff0;
    GPIOA_CRH |= 0x00000001;

    GPIOB_BRR = 0xffffffff;
    GPIOB_CRL = 0x22222222;
    GPIOB_CRH = 0x22222222;

    GPIOC_CRH &= 0x0fffffff;
    GPIOC_CRH |= 0x20000000;

    //
    AFIO_MAPR &= (~(0x01 << 0));

    // Set GND on Pin 4
    GPIOB_BSRR = (1 << 3);

    // Set VCC on Pin 8
    GPIOB_BSRR = (1 << 15);

    // Turn on VCC 3.3V
    GPIOC_BSRR = (1 << 15);

    // Set High on Pin 3 and 7
    GPIOA_BSRR = ((1 << 0) | (1 << 2));
}

//==========================================================================
// Do Command - CMD_ECHO
//==========================================================================
static void DoCmdEcho (int aLen)
{
    int i;

    if (aLen >= 0)
    {
        for (i = 1; i < aLen; i ++)
            gBuf[i] ^= 0xff;

        sys_link_write (gBuf, aLen);
    }
}

//==========================================================================
// Do Command - CMD_GET_INFO
//==========================================================================
static void DoCmdGetInfo (int aLen)
{
    int len;
    unsigned char *ptr;

    ptr = gBuf;
    ptr += PackU8 (ptr, CMD_GET_INFO);
    ptr += PackString (ptr, KName, 16);
    ptr += PackU16 (ptr, KVersion);
    ptr += PackU32 (ptr, 0xffffffff);
    ptr += PackU32 (ptr, 0xffffffff);
    ptr += PackU32 (ptr, 0xffffffff);
    ptr += PackU32 (ptr, 0xffffffff);
    len = ptr - gBuf;

    sys_link_write (gBuf, len);
}

//==========================================================================
// Do Command - CMD_EXIT
//==========================================================================
static void DoCmdExit (int aLen)
{
    int len;
    unsigned char *ptr;

    ptr = gBuf;
    ptr += PackU8 (ptr, CMD_EXIT);
    ptr += PackU8 (ptr, 0x01);
    len = ptr - gBuf;

    sys_link_write (gBuf, len);
}

//==========================================================================
// Do Command - CMD_CHIP_INFO
//==========================================================================
static void DoCmdChipInfo (int aLen)
{
    unsigned long id;

    //
    if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
        id = 0xffffff;
    else
        id = SflashReadId ();

    //
    gBuf[0] = CMD_CHIP_INFO;
    gBuf[1] = 0x01;
    gBuf[2] = (id >> 24);
    gBuf[3] = (id >> 16);
    gBuf[4] = (id >> 8);
    gBuf[5] = (id >> 0);

    sys_link_write (gBuf, 6);
}

//==========================================================================
// Do Command - CMD_CHIP_ERASE
//==========================================================================
static void DoCmdChipErase (int aLen)
{
    unsigned long timeout;

    if (aLen >= 5)
    {
        timeout = UnpackU32 (&gBuf[1]);
    }
    else
    {
        timeout = TIMEOUT_ERASE;
    }

    //
    gBuf[1] = 0x00;
    gBuf[2] = 0xff;
    gBuf[3] = 0xff;

    do
    { 
        // Wait ready
        if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
            break;

        //
        SflashReset ();

        // Do Erase
        SflashWriteEnable ();
        SflashChipErase ();
        gReady = false;

        // Wait ready
        if (WaitSflashReady (timeout) < 0)
            break;

        // Set result to success
        gBuf[1] = 0x01;
        gBuf[2] = SflashReadStatus1 ();
        gBuf[3] = SflashReadStatus2 ();
    } while (0);
    SpiSetCs (1);

    //
    gBuf[0] = CMD_CHIP_ERASE;

    sys_link_write (gBuf, 4);
}

//==========================================================================
// Do Command - CMD_READ_DATA
//==========================================================================
static void DoCmdReadData (int aLen)
{
    unsigned long addr;

    // Get Addr
    addr = UnpackU32 (&gBuf[1]);

    //
    gBuf[1] = 0x00;

    do
    {
        // Check len
        if (aLen != 5)
        {
            for (addr = 0; addr < 1024; addr ++)
                gBuf[2 + addr] = 0xff;
            break;
        }

        // Check Addr
        if (addr & 0x03ff)
        {
            for (addr = 0; addr < 1024; addr ++)
                gBuf[2 + addr] = 0xff;
            break;
        }

        // Wait ready
        if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
            break;

        // Read Data
        SflashReadPage (addr, &gBuf[2 + 0]);
        addr += 256;
        SflashReadPage (addr, &gBuf[2 + 256]);
        addr += 256;
        SflashReadPage (addr, &gBuf[2 + 512]);
        addr += 256;
        SflashReadPage (addr, &gBuf[2 + 768]);

        // Set result to success
        gBuf[1] = 0x01;
    } while (0);
    SpiSetCs (1);

    //
    gBuf[0] = CMD_READ_DATA;

    sys_link_write (gBuf, (1024 + 4));

}


//==========================================================================
// Do Command - CMD_WRITE_DATA
//==========================================================================
static void DoCmdWriteData (int aLen)
{
    unsigned long addr;
    unsigned char *ptr;
    int i;

    // Get Addr
    addr = UnpackU32 (&gBuf[1]);

    //
    gBuf[1] = 0x00;

    do
    {
        // Check len
        if (aLen != (1024 + 5))
            break;

        // Check Addr
        if (addr & 0x03ff)
            break;

        //
        ptr = &gBuf[5];
        for (i = 4; i; i --)
        {
            // Wait ready
            if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
                break;

            // Do Write
            SflashWriteEnable ();
            SflashWritePage (addr, ptr);
            gReady = false;

            //
            addr += 256;
            ptr += 256;
        }

        // Error break from for-loop
        if (i)
            break;

        // Set result to success
        gBuf[1] = 0x01;
    } while (0);
    SpiSetCs (1);

    //
    gBuf[0] = CMD_WRITE_DATA;

    sys_link_write (gBuf, 2);

}

//==========================================================================
// Do Command - CMD_BLANK_CHECK
//==========================================================================
static void DoCmdBlankCheck (int aLen)
{
    unsigned long addr;
    unsigned long size;
    unsigned long l;
    int i;

    // Get Addr
    addr = UnpackU32 (&gBuf[1]);
    size = UnpackU32 (&gBuf[5]);

    //
    gBuf[1] = 0x00;

    do
    {
        // Check len
        if (aLen != 9)
        {
            for (addr = 0; addr < 512; addr ++)
                gBuf[2 + addr] = 0xff;
            break;
        }

        // Check Addr
        if ((addr & 0x01ff) || (size & 0x01ff))
        {
            for (addr = 0; addr < 512; addr ++)
                gBuf[2 + addr] = 0xff;
            break;
        }

        // Wait ready
        if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
            break;

        // Read Data
        while (size)
        {
            SflashReadPage (addr, &gBuf[2 + 0]);
            for (i = 0; i < 256; i ++)
            {
                if (gBuf[2 + i] != 0xff)
                    break;
            }
            if (i != 256)
                break;
            addr += 256;
            size -= 256;

            //
            l = addr & 0xffff;
            if (l == 0)
                sys_led0_on ();
            else if (l == 0x8000)
                sys_led0_off ();
        }
        if (size)
            break;

        // Set result to success
        gBuf[1] = 0x01;
    } while (0);
    SpiSetCs (1);

    //
    gBuf[0] = CMD_BLANK_CHECK;

    sys_link_write (gBuf, 2);

}

//==========================================================================
// Do Read Status Reg - CMD_READ_REG
//==========================================================================
static void DoCmdReadStatus (int aLen)
{
    unsigned char ret = 0;
    unsigned char status1;
    unsigned short status2;

    //
    if ((aLen == 2) && (gBuf[1] == 2))
    {
        // Read 2 bytes status
        if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
            status2 = 0xffff;
        else
        {
            status2 = SflashReadStatus2 ();
            ret = 1;
        }

        //
        gBuf[0] = CMD_READ_REG;
        gBuf[1] = ret;
        gBuf[2] = (status2 >> 8);
        gBuf[3] = (status2 >> 0);
        sys_link_write (gBuf, 4);
    }
    else
    {
        // Read 1 byte status
        if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
            status1 = 0xff;
        else
        {
            status1 = SflashReadStatus1 ();
            ret = 1;
        }

        //
        gBuf[0] = CMD_READ_REG;
        gBuf[1] = ret;
        gBuf[2] = status1;
        sys_link_write (gBuf, 3);
    }
}

//==========================================================================
// Do Write Status Reg - CMD_WRITE_REG
//==========================================================================
static void DoCmdWriteStatus (int aLen)
{
    unsigned char status;
    unsigned char ret;

    //
    status = 0xff;
    ret = 0;

    // Get Addr
    if (aLen  == 2)
    {
        if (WaitSflashReady (TIMEOUT_DEFAULT) >= 0)
        {
            SflashWriteEnable ();
            SflashWriteStatus1 (gBuf[1]);
            status = SflashReadStatus1 ();
            ret = 1;
        }
    }

    //
    gBuf[0] = CMD_WRITE_REG;
    gBuf[1] = ret;
    gBuf[2] = status;

    sys_link_write (gBuf, 3);
}

//==========================================================================
// Do Enter 4-Byte Addr Mode - CMD_ENTER_4BYTE_ADDR
//==========================================================================
static void DoEnter4ByteAddrMode (int aLen)
{
    unsigned char ret;

    if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
    {
        ret = 0;
    }
    else
    {
        Sflash4ByteAddrMode (true);
        ret = 1;
    }

    //
    gBuf[0] = CMD_ENTER_4BYTE_ADDR;
    gBuf[1] = ret;

    sys_link_write (gBuf, 2);
}

//==========================================================================
// Do Exit 4-Byte Addr Mode - CMD_ENTER_4BYTE_ADDR
//==========================================================================
static void DoExit4ByteAddrMode (int aLen)
{
    unsigned char ret;

    if (WaitSflashReady (TIMEOUT_DEFAULT) < 0)
    {
        ret = 0;
    }
    else
    {
        Sflash4ByteAddrMode (false);
        ret = 1;
    }

    //
    gBuf[0] = CMD_EXIT_4BYTE_ADDR;
    gBuf[1] = ret;

    sys_link_write (gBuf, 2);
}

//==========================================================================
// Main Entry Point
//==========================================================================
void main()
{
    int i;
    unsigned short version;
    unsigned long tick_exit;
    unsigned long tick;
 
    //
    IoInit ();
    SpiInit ();
    SflashInit ();

    //
    version = sys_version ();

    // Power up waiting
    tick = GetTick ();
    while (TickElapsed (tick) < 100)
        asm ("nop;");

    //
    tick_exit = 0;
    gReady = false;
    while (1)
    {
        if (TickElapsed (gTickLed) < 200)
        {
            if (GetTick () & 0x40)
                sys_led0_on ();
            else 
                sys_led0_off ();
        }
        else
        {
            sys_led0_on ();
        }

        // Link process
        sys_link_process ();

        // Get Command
        i = sys_link_read (gBuf, sizeof (gBuf));
        if (i < 0)
            break;
        gTickLed = GetTick ();
        if (i)
        {
            switch (gBuf[0])
            {
                case CMD_ECHO:
                    DoCmdEcho (i);
                    break;
        
                case CMD_GET_INFO:
                    DoCmdGetInfo (i);
                    break;

                case CMD_EXIT:
                    DoCmdExit (i);
                    tick_exit = GetTick ();
                    if (tick_exit == 0)
                        tick_exit = 1;
                    break;

                case CMD_CHIP_INFO:
                    DoCmdChipInfo (i);
                    break;

                case CMD_CHIP_ERASE:
                    DoCmdChipErase (i);
                    break;

                case CMD_READ_DATA:
                    DoCmdReadData (i);
                    break;

                case CMD_WRITE_DATA:
                    DoCmdWriteData (i);
                    break;

                case CMD_READ_REG:
                    DoCmdReadStatus (i);
                    break;

                case CMD_WRITE_REG:
                    DoCmdWriteStatus (i);
                    break;

                case CMD_BLANK_CHECK:
                    DoCmdBlankCheck (i);
                    break;

                case CMD_ENTER_4BYTE_ADDR:
                    DoEnter4ByteAddrMode (i);
                    break;

                case CMD_EXIT_4BYTE_ADDR:
                    DoExit4ByteAddrMode (i);
                    break;

            }
        }

        // Exiting
        if (tick_exit)
        {
            // Turn off VCC 3.3V
            GPIOC_BRR = (1 << 15);

            //
            if (TickElapsed (tick_exit) > 100)
            {
                // Release all IO
                GPIOB_BRR = 0xffffffff;
                GPIOB_CRL = 0x44444444;
                GPIOB_CRH = 0x44444444;

                GPIOC_CRH &= 0x00ffffff;
                GPIOC_CRH |= 0x44000000;

                // Return back to BaseSystem
                sys_exit_user ();
            }
        }
    }
}

