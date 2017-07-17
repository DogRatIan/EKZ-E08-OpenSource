//==========================================================================
// Timer 1
//==========================================================================
#include "mcu.h"
#include "int.h"
#include "timer1.h"

//==========================================================================
// Global Variables
//==========================================================================
static unsigned long gTick;

//==========================================================================
// Timer1 Handler
//==========================================================================
void TIM1_UP_IRQHandler (void)
{
//    // Test pin
//    GPIOB_BSRR = (1 << 0);
//    GPIOB_BRR = (1 << 0);

    // Free running tick counter
    gTick++;

    // Clear interrupt flag
    TIM1_SR &= (~(0x01 << 0));

    //
    NVIC_ClrPend (NVIC_TIM1_UP);
}

//==========================================================================
// Timer1 Init - 1ms
//==========================================================================
void Timer1Init (void)
{
    //
    gTick = 0;

    // Enable clock and release reset on TIM1
    RCC_APB2RSTR &= (~(1 << 11));
    RCC_APB2ENR  |=   ( 1 << 11);

    // Setup Timer
    TIM1_PSC    = 63;
    TIM1_ARR    = 1124;
    TIM1_CCR1   = 562;
    TIM1_CR1    = 0x0004;
    TIM1_CCMR1  = 0x0060;
    TIM1_CCER   = 0x0001;
    TIM1_BDTR   = 0x8044;
    TIM1_SR     = 0x0000;

    // Enable Interrupt
    TIM1_DIER   = 0x0001;
    NVIC_IntEnable (NVIC_TIM1_UP);

    // Start the timer
    TIM1_CR1 |= 0x01;
}

//==========================================================================
// Tick Handling
//==========================================================================
unsigned long GetTick (void)
{
    return gTick;
}

//==========================================================================
// Tick Elapsed
//==========================================================================
unsigned long TickElapsed (unsigned long aTick)
{
    unsigned long curr_tick;

    curr_tick = gTick;
    if (curr_tick >= aTick)
        return (curr_tick - aTick);
    else
        return (0xffffffff - aTick + curr_tick);
}

