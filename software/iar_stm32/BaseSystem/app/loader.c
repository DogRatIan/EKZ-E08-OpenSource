//==========================================================================
// USBDog Loader
//==========================================================================
#include "includes.h"
#include "printf2.h"
#include "uart1.h"
#include "timer1.h"

#include "crc16.h"
#include "crc8.h"
#include "var.h"
#include "packer.h"
#include "system.h"
#include "ekz_misc.h"

#include "loader.h"

//==========================================================================
// Defines
//==========================================================================
enum
{
    S_RX_IDLE,
    S_RX_CRC8,
    S_RX_LEN_MSB,
    S_RX_LEN_LSB,
    S_RX_DATA,
    S_RX_ERROR,
};

enum
{
    S_TX_IDLE,
    S_TX_WAIT,    
    S_TX_HEADER,
    S_TX_DATA,
    S_TX_ERROR,
};

// Special Character
#define CHAR_SOH            0x01

//
#define VALUE_CRC8_IV       0x55

// Command List (Common)
#define CMD_GET_INFO        0x00
#define CMD_ECHO            0xff
#define RESP_ERROR          0x80

// Command List BaseSystem
#define CMD_CLR_UPROG       0x81
#define CMD_LOAD_UPROG      0x82
#define CMD_CHK_UPROG       0x83
#define CMD_START_UPROG     0x84

//
#define CMD_EXIT            0x01

// Timing
#define TIMEOUT_INTERCHAR       100         //100ms

//
#define ADDR_USER_PROGRAM       0x20002000
#define SIZE_USER_PROGRAM_MAX   0x2000

//==========================================================================
// Constants
//==========================================================================
static const char *KName="EKZ-E08";
static const unsigned short KVersion = 0x0200;

//==========================================================================
// Variables
//==========================================================================
#pragma data_alignment=4
static unsigned char gRxedBuf[1280];

#pragma data_alignment=4
static unsigned char gTxBuf[1280];

static char gRxState;
static unsigned long gRxTick;
static int gRxingLen;
static int gRxingIdx;
static unsigned char gRxingCrc;
static unsigned char gRxingCheck;
static int gRxedLen;
static int gTxLen;

static int gStartUserProg;

static unsigned long gUserProgramSize;

//==========================================================================
// Pack Communication header
//==========================================================================
static inline int PackDummyHeader (unsigned char *aDest)
{
    return PackU32 (aDest, 0);
}

//==========================================================================
// Fix Header
//==========================================================================
static inline void FixHeader (unsigned char *aDest, int aLen)
{
    unsigned char crc;
    unsigned char *ptr;
    int len;

    if (aLen < 4)
        return;

    len = aLen - 4;

    aDest[0] = CHAR_SOH;
    aDest[2] = (len >> 8);
    aDest[3] = (len >> 0);

    // Cal CRC from LEN to Data Len
    ptr = &aDest[2];
    crc = VALUE_CRC8_IV;
    len = aLen - 2;
    while (len)
    {
        crc = Crc8 (crc, *ptr);
        ptr ++;
        len --;
    }
    aDest[1] = crc;
}


//==========================================================================
// RX Process
//==========================================================================
static inline void RxProcess (void)
{
    unsigned char rx_len;
    unsigned char i;
    unsigned char ch;
    
    // Get data
    rx_len = UsbCdcRead (gBuf, sizeof(gBuf) - 1);
    
    // Inter-char timeout handle
    if (rx_len <= 0)
    {
        if (TickElapsed (gRxTick) > TIMEOUT_INTERCHAR)
            gRxState = S_RX_IDLE;
        return;
    }
    gRxTick = GetTick ();
  
    // Process on rxed data
    for (i = 0; i < rx_len; i ++)
    {
        ch = gBuf[i];

        // Cal Check
        gRxingCheck = Crc8 (gRxingCheck, ch);

        switch (gRxState)
        {
            case S_RX_IDLE:
                // Rxed buffer not handle
                if (gRxedLen)
                {
                    gRxState = S_RX_ERROR;
                    break;
                }
                if (ch == CHAR_SOH)
                {
                    gRxState = S_RX_CRC8;
                    break;
                }
                break;

            case S_RX_CRC8:
                gRxingCrc = ch;
                gRxingCheck = VALUE_CRC8_IV;
                gRxState = S_RX_LEN_MSB;
                break;

            case S_RX_LEN_MSB:
                gRxingLen = ch << 8;
                gRxState = S_RX_LEN_LSB;
                break;

            case S_RX_LEN_LSB:
                gRxingLen |= ch;
                gRxingIdx = 0;
                gRxState = S_RX_DATA;
                break;

            case S_RX_DATA:
                if (gRxingIdx >= sizeof (gRxedBuf))
                {
                    gRxState = S_RX_ERROR;
                    break;
                }
                gRxedBuf[gRxingIdx] = ch;
                gRxingIdx ++;
                if (gRxingIdx == gRxingLen)
                {
                    // Check XOR/CRC
                    if (gRxingCheck != gRxingCrc)
                    {
                        gRxState = S_RX_ERROR;
                        break;
                    }

                    // Check  ADDR
                    gRxedLen = gRxingIdx;
                    gRxState = S_RX_IDLE;
                }
                break;

            default:
                gRxState = S_RX_IDLE;
                break;
        }
    }
}

//==========================================================================
// Send Error Response
//==========================================================================
static void SendErrorResp (const char *aMsg)
{
    unsigned char *ptr;
    int msg_len;

    // Set unknown message
    if (aMsg == NULL)
        aMsg = "Unknown Error";

    // Limit message length
    msg_len = strlen (aMsg);
    if (msg_len > 128)
        msg_len = 128;

    ptr = gTxBuf;
    ptr += PackDummyHeader (ptr);
    ptr += PackU8 (ptr, RESP_ERROR);
    ptr += PackString (ptr, aMsg, msg_len);
    ptr += PackU8 (ptr, 0x00);
    gTxLen = ptr - gTxBuf;
    FixHeader (gTxBuf, gTxLen);
}

//==========================================================================
// Send Result
//==========================================================================
static void SendResult (unsigned char aCmd, unsigned char aResult)
{
    unsigned char *ptr;

    ptr = gTxBuf;
    ptr += PackDummyHeader (ptr);
    ptr += PackU8 (ptr, aCmd);
    ptr += PackU8 (ptr, aResult);
    gTxLen = ptr - gTxBuf;
    FixHeader (gTxBuf, gTxLen);
}

//==========================================================================
// Do Cmd - CMD_ECHO
//==========================================================================
static inline void DoCmdEcho (void)
{
    int i;
    unsigned char *ptr;

    if (gRxedLen >= 0)
    {
        ptr = gTxBuf;
        ptr += PackDummyHeader (ptr);
        ptr += PackU8 (ptr, CMD_ECHO);
        for (i = 1; i < gRxedLen; i ++)
            ptr += PackU8 (ptr, ~gRxedBuf[i]);
        gTxLen = ptr - gTxBuf;
        FixHeader (gTxBuf, gTxLen);
    }
}

//==========================================================================
// Do Cmd - CMD_GET_INFO
//==========================================================================
__IO_REG32      (U_ID0,  0x1FFFF7E8, __READ);
__IO_REG32      (U_ID1,  0x1FFFF7EC, __READ);
__IO_REG32      (U_ID2,  0x1FFFF7F0, __READ);


static inline void DoCmdGetInfo (void)
{
    unsigned char *ptr;

    ptr = gTxBuf;
    ptr += PackDummyHeader (ptr);
    ptr += PackU8 (ptr, CMD_GET_INFO);
    ptr += PackString (ptr, KName, 16);
    ptr += PackU16 (ptr, KVersion);
    ptr += PackU32 (ptr, U_ID0);
    ptr += PackU32 (ptr, U_ID1);
    ptr += PackU32 (ptr, U_ID2);
    ptr += PackU32 (ptr, 0xffffffff);
    gTxLen = ptr - gTxBuf;
    FixHeader (gTxBuf, gTxLen);
}

//==========================================================================
// Do Cmd - CMD_CLR_UPROG
//==========================================================================
static inline void DoCmdClrUserProg (void)
{
    unsigned char *ptr;
    unsigned long l;
    
    // Clear User program area
    ptr = (unsigned char *)ADDR_USER_PROGRAM;
    for (l = SIZE_USER_PROGRAM_MAX; l; l --)
    {
        *ptr = 0xff;
        ptr ++;
    }
    gUserProgramSize = 0;
            
    // Send Resp
    SendResult (CMD_CLR_UPROG, 1);
}

//==========================================================================
// Do Cmd - CMD_LOAD_UPROG
//==========================================================================
static inline void DoCmdLoadUserProg (void)
{
    unsigned char *dest_ptr;
    unsigned long l;
    unsigned long addr;
    int i;
    unsigned char *ptr;
    unsigned char crc8;
    int decode_idx;
    
    // Check data length
    if (gRxedLen != 517)
    {
        SendResult (CMD_LOAD_UPROG, 0);
        return;
    }

    // Check offset
    addr = UnpackU32 (&gRxedBuf[1]);
    if (addr > (SIZE_USER_PROGRAM_MAX - 512))
    {
        SendResult (CMD_LOAD_UPROG, 0);
        return;
    }
    l = addr + ADDR_USER_PROGRAM;
    dest_ptr = (unsigned char *)l;

    //Decode data
    ptr = &gRxedBuf[5];
    for (i = 0; i < 512; i ++)
    {
        decode_idx = addr + i;
        crc8 = Crc8 (0x55, decode_idx);
        ptr[i] ^= crc8;
    }

    // Copy data
    memcpy (dest_ptr, &gRxedBuf[5], 512);
    addr += 512;
    if (gUserProgramSize < addr)
        gUserProgramSize = addr;
    printf ("gUserProgramSize=%d\n", gUserProgramSize);
            
    // Send Resp
    SendResult (CMD_LOAD_UPROG, 1);
}

//==========================================================================
// Do Cmd - CMD_CHK_UPROG
//==========================================================================
static inline void DoCmdCheckUserProg (void)
{
    unsigned short crc_cmd;
    unsigned short crc_cal;
    const unsigned char *ptr;
    int i;

    // Check data length
    if (gRxedLen != 3)
    {
        SendResult (CMD_CHK_UPROG, 0);
        return;
    }

    // Get CRC
    crc_cmd = UnpackU16 (&gRxedBuf[1]);

    // Cal CRC
    crc_cal = 0x8888;
    ptr = (const unsigned char *)ADDR_USER_PROGRAM;
    for (i = 0; i < gUserProgramSize; i ++)
    {
        crc_cal = Crc16Ccitt (crc_cal, *ptr);
        ptr ++;
    }
    printf (" CRC=0x%04X\n", crc_cal);

    //
    if ((crc_cal != crc_cmd) || (gUserProgramSize == 0))
        SendResult (CMD_CHK_UPROG, 0);
    else
        SendResult (CMD_CHK_UPROG, 1);
}

//==========================================================================
// Do Cmd - CMD_START_UPROG
//==========================================================================
static inline void DoCmdStartUserProg (void)
{
    unsigned short crc_cmd;
    unsigned short crc_cal;
    const unsigned char *ptr;
    int i;

    // Check data length
    if (gRxedLen != 3)
    {
        SendResult (CMD_START_UPROG, 0);
        return;
    }

    // Get CRC
    crc_cmd = UnpackU16 (&gRxedBuf[1]);

    // Cal CRC
    crc_cal = 0x8888;
    ptr = (const unsigned char *)ADDR_USER_PROGRAM;
    for (i = 0; i < gUserProgramSize; i ++)
    {
        crc_cal = Crc16Ccitt (crc_cal, *ptr);
        ptr ++;
    }
    printf ("CRC=0x%04X\n", crc_cal);

    //
    if ((crc_cal != crc_cmd) || (gUserProgramSize == 0))
        SendResult (CMD_START_UPROG, 0);
    else
    {
        SendResult (CMD_START_UPROG, 1);
        gStartUserProg = true;
    }
}


//==========================================================================
// Command process
//==========================================================================
static void CmdProcess (void)
{
    // Any Txing
    if (gTxLen)
        return;

    // Any rxed
    if (gRxedLen == 0)
        return;

    //
    Led0Off ();

    // Handle commands
    switch (gRxedBuf[0])
    {
        case CMD_ECHO:
            DoCmdEcho ();
            break;

        case CMD_GET_INFO:
            DoCmdGetInfo ();
            break;

        case CMD_CLR_UPROG:
            DoCmdClrUserProg ();
            break;

        case CMD_LOAD_UPROG:
            DoCmdLoadUserProg ();
            break;

        case CMD_CHK_UPROG:
            DoCmdCheckUserProg ();
            break;

        case CMD_START_UPROG:
            DoCmdStartUserProg ();
            break;

        case CMD_EXIT:
            SendResult (CMD_EXIT, 1);
            break;

        default:
            SendErrorResp ("Unknown Command");
            break;
    }
    
    // Reset for next command
    gRxingIdx = 0;
    gRxedLen = 0;
}

//==========================================================================
// ^^^^^^^^^^^^^^^
// Local routines
//==========================================================================
//==========================================================================
//==========================================================================
//==========================================================================
// Public routines
// vvvvvvvvvvvvvvv
//==========================================================================

//==========================================================================
// Initial Loader
//==========================================================================
void LoaderInit (void)
{
    //
    gStartUserProg = false;
    gRxingCrc = 0;
    gRxState = S_RX_IDLE;
    gRxedLen = 0;
    gTxLen = 0;
}

//==========================================================================
// Process CDC in/out data
//==========================================================================
void LoaderProcess (void)
{
    // TxProcess
    if (gTxLen)
    {
        if (!UsbCdcWrite (gTxBuf, gTxLen))
        {
            return;
        }
        gTxLen = 0;
        if (gStartUserProg)
        {
            sys_enter_user ();
            gStartUserProg = false;
        }
    }

    //
    RxProcess ();
    CmdProcess ();
    Led0On ();
}

//==========================================================================
// Link layer process
//  Return -ve value when there is failure, 
//  User program should exit and back to system when it happen.
//==========================================================================
int sys_link_process (void)
{
    // Check disconnected
    if (!IsUsbCdcConfigure())
        return -1;

    // TxProcess
    if (gTxLen)
    {
        if (!UsbCdcWrite (gTxBuf, gTxLen))
        {
            return 0;
        }
        gTxLen = 0;
    }

    //
    RxProcess ();
    return 0;
}

//==========================================================================
// Get received command
//==========================================================================
int sys_link_read (unsigned char *aDest, int aLen)
{
    if (gRxedLen <= 0)
        return 0;
    
    if (aLen > gRxedLen)
        aLen = gRxedLen;
    memcpy (aDest, gRxedBuf, aLen);
    gRxedLen = 0;

    return aLen;
}

//==========================================================================
// Send out response
//==========================================================================
int sys_link_write (const unsigned char *aSrc, int aLen)
{
    unsigned char *ptr;

    if (gTxLen > 0)
        return -1;
    if (aLen > 1276)
        return -1;
    
    ptr = gTxBuf;
    ptr += PackDummyHeader (ptr);
    memcpy (ptr, aSrc, aLen);
    ptr += aLen;
    gTxLen = ptr - gTxBuf;
    FixHeader (gTxBuf, gTxLen);

    return 0;
}
