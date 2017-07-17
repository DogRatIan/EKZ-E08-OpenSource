//==========================================================================
// SPI
//==========================================================================
#include "mcu.h"
#include "spi.h"

//==========================================================================
// Defines
//==========================================================================
#define SetCs0High()    GPIOB_BRR  = (1 << 0)
#define SetCs0Low()     GPIOB_BSRR = (1 << 0)

#define SetSckHigh()    GPIOB_BRR  = (1 << 5)
#define SetSckLow()     GPIOB_BSRR = (1 << 5)
//#define SetSckHigh()    {GPIOB_BRR  = (1 << 5); GPIOA_BSRR = (1 << 5); GPIOA_CRL |= 0x00300000;}
//#define SetSckLow()     {GPIOA_CRL &= 0xff0fffff; GPIOB_BSRR = (1 << 5);}

#define SetMosiHigh()    GPIOB_BRR  = (1 << 4)
#define SetMosiLow()     GPIOB_BSRR = (1 << 4)
//#define SetMosiHigh()   {GPIOB_BRR  = (1 << 4); GPIOA_BSRR = (1 << 7); GPIOA_CRL |= 0x30000000;}
//#define SetMosiLow()    {GPIOA_CRL &= 0x0fffffff; GPIOB_BSRR = (1 << 4);}

#define GetMiso()       (GPIOA_IDR & (1 << 6))

//==========================================================================
// Send and Receive 1 Byte
//==========================================================================
int SpiByte (unsigned char aCh)
{
    unsigned char ch;
    int i;
    volatile int delay;

    ch = 0xff;
    for (i = 0; i < 8; i ++)
    {

        // Ready Data out
        if (aCh & 0x80)
        {
            SetMosiHigh ();
        }
        else
        {
            SetMosiLow ();
        }
        aCh = (aCh << 1);

        // Clock Falling Edge
        SetSckLow ();
        for (delay = 7; delay; delay --);

        // Sample Data in
        ch = (ch << 1);
        if (GetMiso () != 0)
            ch |= 0x01;
        
        // Clock Rising Edge
        SetSckHigh ();
        for (delay = 10; delay; delay --);
    }

    return ch;
}

//==========================================================================
// Set nCS
//==========================================================================
void SpiSetCs (int aState)
{
    volatile int delay;
    if (aState)
        SetCs0High ();
    else
        SetCs0Low ();
    for (delay = 30; delay; delay --);
}

//==========================================================================
// Init SPI0
//==========================================================================

void SpiInit(void)
{
    // Init nCS pin (GPIO)
    SetCs0High ();

    SetSckHigh ();
}
