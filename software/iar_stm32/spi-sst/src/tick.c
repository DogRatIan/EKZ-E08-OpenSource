//==========================================================================
//==========================================================================
#include "system.h"

#include "tick.h"


//==========================================================================
// Tick Handling
//==========================================================================
unsigned long GetTick (void)
{
    return sys_get_tick ();
}

//==========================================================================
// Tick Elapsed
//==========================================================================
unsigned long TickElapsed (unsigned long aTick)
{
    unsigned long curr_tick;

    curr_tick = sys_get_tick ();
    if (curr_tick >= aTick)
        return (curr_tick - aTick);
    else
        return (0xffffffff - aTick + curr_tick);
}

