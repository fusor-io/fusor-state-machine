#ifndef varstruct_h
#define varstruct_h

#include "math.h"

#define VAR_TYPE_FLOAT 0
#define VAR_TYPE_LONG 1

typedef struct VarStruct
{
    VarStruct(long int _value)
        : type(VAR_TYPE_LONG), vFloat((float)_value), vInt(_value) {}
    VarStruct(float _value)
        : type(VAR_TYPE_LONG), vFloat(_value), vInt(round(_value)) {}

    char type;
    volatile long int vInt;
    volatile float vFloat;
} VarStruct;

#endif