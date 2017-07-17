/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : main.c
 *    Description : Define main module
 *
 *    History :
 *    1. Date        : 19, July 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *  This example project shows how to use the IAR Embedded Workbench
 * for ARM to develop code for the IAR STM32-SK board.
 *  It implements USB CDC (Communication Device Class) device and install
 * it like a Virtual COM port. UART3 is used for physical implementation
 * of the RS232 port.
 *
 *  Jumpers:
 *   PWR_SEL - depending of power source
 *
 *    $Revision: 38757 $
 **************************************************************************/
#include "includes.h"
#include "ekz_misc.h"
#include "timer1.h"
#include "uart1.h"
#include "printf2.h"
#include "loader.h"
#include "spi.h"

#define DLY_100US  450

Int32U CriticalSecCntr;

/*************************************************************************
 * Function Name: DelayResolution100us
 * Parameters: Int32U Dly
 *
 * Return: none
 *
 * Description: Delay ~ (arg * 100us)
 *
 *************************************************************************/
void DelayResolution100us(Int32U Dly)
{
    for(; Dly; Dly--)
    {
        for(volatile Int32U j = DLY_100US; j; j--)
        {
        }
    }
}

/*************************************************************************
 * Function Name: main
 * Parameters: none
 *
 * Return: none
 *
 * Description: main
 *
 *************************************************************************/
__IO_REG32      (SCB_VTOR,  0xE000ED08, __READ_WRITE);
typedef void (* func)(void);

void main(void)
{
    volatile int i;

    //
    ENTR_CRT_SECTION();

    /* Setup STM32 system (clock, PLL and Flash configuration) */
    SystemInit();

    // NVIC init
    #ifndef  EMB_FLASH
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
    #else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
    #endif
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // Initialization
    IoInit ();
    UsbDisconnect ();
    Timer1Init ();
    Uart1Init ();
//    SpiInit ();
    LoaderInit ();

    //
    for (i = 50000; i; i --);
    UsbConnect ();

    // CDC USB
    UsbCdcInit();

    // Soft connection enable
//    USB_ConnectRes(TRUE);

    EXT_CRT_SECTION();

    Led0Off ();

    // Main loop
    printf ("Started\n");
    while(1)
    {
        // Check USB connected and ready
        if (!IsUsbCdcConfigure())
        {
//            // Slow blinking on LED if no USB
//            if (GetTick () & 0x0000400)
//                Led0On ();
//            else
//                Led0Off ();
            continue;
        }

//        // Fast blinking on LED if USB ready
//        if (GetTick () & 0x0000100)
//            Led0On ();
//        else
//            Led0Off ();

        // Loader process
        LoaderProcess ();
    }
}
