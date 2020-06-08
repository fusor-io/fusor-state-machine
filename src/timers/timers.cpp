#include <map>
#include <limits.h>

#include "../keycompare/keycompare.h"
#include "timers.h"

std::map<const char *, TIMER_SLOT, KeyCompare> Timers::_timerMap;

Timers::Timers(GetTimeFunction getTime)
{
    _getTimeCallback = getTime;
}

/**
 * Get local time
 * @return local time in ms (ticks)
 */
unsigned long Timers::getTime()
{
    return _getTimeCallback ? _getTimeCallback() : 0;
}

/**
 * Chack if timer elapsed. If elapsed, return true and remove timer
 * @param timerName unique timer name
 * @param timeout milliseconds
 * @return true if amount timeout ms passed since timer initialized
 */
bool Timers::validateTimer(const char *timerName, unsigned long timeout)
{
    if (!_getTimeCallback)
        return true;

    // check if timer is already registered
    if (_timerMap.count(timerName))
    {
        TIMER_SLOT slot = _timerMap[timerName];

        // has it already elapsed on the last check?
        if (slot.isElapsed)
        {
            _timerMap[timerName] = {getTime(), false};
            return false;
        }
        else
        {
            if (elapsed(slot.startTime, getTime()) >= timeout)
            {
                _timerMap[timerName].isElapsed = true;
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        _timerMap[timerName] = {getTime(), false};
        return false;
    }
}

unsigned long Timers::elapsed(unsigned long start, unsigned long end)
{
    // we should handle time overflow condition
    // if end < start it means we had overflow
    return end >= start ? end - start : ULONG_MAX - start + 1 + end;
}
