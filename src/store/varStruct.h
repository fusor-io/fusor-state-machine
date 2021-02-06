#ifndef varstruct_h
#define varstruct_h

#include "math.h"

#define VAR_TYPE_FLOAT 0
#define VAR_TYPE_LONG 1

typedef struct VarStruct
{
    VarStruct()
        : type(VAR_TYPE_FLOAT), vFloat(0.0f), vInt(0) {}
    VarStruct(long int _value)
        : type(VAR_TYPE_LONG), vFloat((float)_value), vInt(_value) {}
    VarStruct(float _value)
        : type(VAR_TYPE_FLOAT), vFloat(_value), vInt(round(_value)) {}
    VarStruct(VarStruct *_value)
        : type(_value->type), vFloat(_value->vFloat), vInt(_value->vInt) {}

    char type;
    volatile long int vInt;
    volatile float vFloat;

    void operator=(long int &value)
    {
        type = VAR_TYPE_LONG;
        vInt = value;
        vFloat = (float)value;
    }

    void operator=(int &value)
    {
        type = VAR_TYPE_LONG;
        vInt = value;
        vFloat = (float)value;
    }

    void operator=(float &value)
    {
        type = VAR_TYPE_FLOAT;
        vInt = round(value);
        vFloat = value;
    }
} VarStruct;

#endif