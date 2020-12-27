#ifndef hooks_h
#define hooks_h

#include "../store/varStruct.h"

class Hooks
{
public:
    virtual void onVarUpdate(const char *, VarStruct *);
    virtual void afterCycle(unsigned long);
};

#endif