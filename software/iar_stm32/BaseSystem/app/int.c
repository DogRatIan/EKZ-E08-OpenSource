//==========================================================================
// NVIC
//==========================================================================
#include "mcu.h"
#include "int.h"

//==========================================================================
// Enable interrup at NVIC
//==========================================================================
void NVIC_IntEnable (unsigned long IntNumber)
{
    if ((IntNumber < WWDG) || (IntNumber > NVIC_USB_WAKE_UP))
        return;

    IntNumber -= WWDG;
    if (IntNumber < 32)
    {
        SETENA0 = (1 << IntNumber);
    }
    else if (IntNumber < 64)
    {
        IntNumber -= 32;
        SETENA1 = (1 << IntNumber);
    }
}

//==========================================================================
// Disables interrup at NVIC
//==========================================================================
void NVIC_IntDisable(unsigned long IntNumber)
{
    if ((IntNumber < WWDG) || (IntNumber > NVIC_USB_WAKE_UP))
        return;

    IntNumber -= WWDG;
    if (IntNumber < 32)
    {
        CLRENA0 = (1 << IntNumber);
    }
    else if (IntNumber < 64)
    {
        IntNumber -= 32;
        CLRENA1 = (1 << IntNumber);
    }
}

//==========================================================================
// Clear pending interrupt at NVIC
//==========================================================================
void NVIC_ClrPend(unsigned long IntNumber)
{
    if ((IntNumber < WWDG) || (IntNumber > NVIC_USB_WAKE_UP))
        return;

    IntNumber -= WWDG;
    if (IntNumber < 32)
    {
        CLRPEND0 = (1 << IntNumber);
    }
    else if (IntNumber < 64)
    {
        IntNumber -= 32;
        CLRPEND1 = (1 << IntNumber);
    }
}

//==========================================================================
// Sets Interrupt priority
//==========================================================================
void NVIC_IntPri(unsigned long IntNumber, unsigned char Priority)
{
    volatile unsigned char *ptr;

    if ((IntNumber < WWDG) || (IntNumber > NVIC_USB_WAKE_UP))
        return;

    IntNumber -= WWDG;
    ptr = (unsigned char *)IP0;
    ptr[IntNumber] = Priority;
}
