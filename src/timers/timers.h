#ifndef smtimers_h
#define smtimers_h

#include <map>
#include "../keycompare/keycompare.h"

typedef struct timer_slot
{
    unsigned long startTime;
    bool isElapsed;
} TIMER_SLOT;

typedef unsigned long (*GetTimeFunction)(void);

class Timers
{
public:
    Timers(GetTimeFunction);
    static std::map<const char *, TIMER_SLOT, KeyCompare> _timerMap;

    GetTimeFunction _getTimeCallback;
    unsigned long getTime();
    bool validateTimer(const char *, unsigned long);

    unsigned long elapsed(unsigned long, unsigned long);
};

#endif
