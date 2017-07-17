//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------
#ifndef _INC_VAR_H
#define _INC_VAR_H

//-----------------------------------------------------------------------
#define MAX_WEEKLYSCHEDULE      16
#define MAX_ADHOCSCHEDULE       16

typedef struct {
    unsigned long   sTime;
    unsigned long   eTime;
} dayRange;

typedef struct {
    unsigned long       idCard;
    dayRange            dRange[8];
    unsigned char       rFlag;
}   WeeklySchedule;

typedef struct {
	unsigned long idCard;
	unsigned long sTime;
	unsigned long eTime;
} AdhocSchedule;

//-----------------------------------------------------------------------
extern unsigned char gBuf[256];

//-----------------------------------------------------------------------
#endif
