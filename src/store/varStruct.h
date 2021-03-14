#ifndef varstruct_h
#define varstruct_h

#include <math.h>
#include <string.h>
#include <functional>

#define VAR_TYPE_FLOAT 0
#define VAR_TYPE_LONG 1
#define VAR_TYPE_NAN 2

typedef struct VarStruct
{
    VarStruct()
        : type(VAR_TYPE_LONG), vFloat(0.0f), vInt(0) {}
    VarStruct(long int _value)
        : type(VAR_TYPE_LONG), vFloat((float)_value), vInt(_value) {}
    VarStruct(int _value)
        : type(VAR_TYPE_LONG), vFloat((float)_value), vInt((long int)_value) {}
    VarStruct(float _value)
        : type(VAR_TYPE_FLOAT), vFloat(_value), vInt(round(_value)) {}
    VarStruct(VarStruct *_value)
        : type(_value->type), vFloat(_value->vFloat), vInt(_value->vInt) {}

    char type;
    volatile long int vInt;
    volatile float vFloat;

    static VarStruct NaN()
    {
        VarStruct val;
        val.type = VAR_TYPE_NAN;
        return val;
    }

    char getType(const VarStruct &src)
    {
        if (type == VAR_TYPE_NAN || src.type == VAR_TYPE_NAN)
            return VAR_TYPE_NAN;
        else
            return type == VAR_TYPE_FLOAT || src.type == VAR_TYPE_FLOAT ? VAR_TYPE_FLOAT : VAR_TYPE_LONG;
    }

    VarStruct initType(const VarStruct &src)
    {
        VarStruct result;
        result.type = getType(src);
        return result;
    }

    VarStruct operation(const VarStruct &src, std::function<long int(long int, long int)> fInt, std::function<float(float, float)> fFloat)
    {
        VarStruct result = initType(src);
        if (result.type == VAR_TYPE_LONG)
        {
            result.vInt = fInt(vInt, src.vInt);
            result.vFloat = (float)result.vInt;
        }
        else
        {
            result.vFloat = fFloat(vFloat, src.vFloat);
            result.vInt = round(result.vFloat);
        }

        return result;
    }

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

    void operator=(const VarStruct &value)
    {
        memcpy(this, &value, sizeof(VarStruct));
    }

    VarStruct operator+(const VarStruct &val)
    {
        return operation(
            val, [](long int a, long int b) { return a + b; }, [](float a, float b) { return a + b; });
    }

    VarStruct operator-(const VarStruct &val)
    {
        return operation(
            val, [](long int a, long int b) { return a - b; }, [](float a, float b) { return a - b; });
    }

    VarStruct operator-()
    {
        return VarStruct(0l) - this;
    }

    VarStruct operator*(const VarStruct &val)
    {
        return operation(
            val, [](long int a, long int b) { return a * b; }, [](float a, float b) { return a * b; });
    }

    VarStruct operator/(const VarStruct &val)
    {
        VarStruct result = initType(val);
        if (result.type == VAR_TYPE_LONG)
        {
            if (val.vInt == 0l)
            {
                result.type = VAR_TYPE_NAN;
            }
            else
            {
                result.vInt = vInt / val.vInt;
                result.vFloat = (float)result.vInt;
            }
        }
        else
        {
            if (val.vFloat == 0.0f)
            {
                result.type = VAR_TYPE_NAN;
            }
            else
            {
                result.vFloat = vFloat / val.vFloat;
                result.vInt = round(result.vFloat);
            }
        }
        return result;
    }

    bool operator>(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat > val.vFloat;
        case VAR_TYPE_LONG:
            return vInt > val.vInt;
        default:
            return false;
        }
    }

    bool operator<(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat < val.vFloat;
        case VAR_TYPE_LONG:
            return vInt < val.vInt;
        default:
            return false;
        }
    }

    bool operator>=(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat >= val.vFloat;
        case VAR_TYPE_LONG:
            return vInt >= val.vInt;
        default:
            return false;
        }
    }

    bool operator<=(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat <= val.vFloat;
        case VAR_TYPE_LONG:
            return vInt <= val.vInt;
        default:
            return false;
        }
    }

    bool operator==(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat == val.vFloat;
        case VAR_TYPE_LONG:
            return vInt == val.vInt;
        default:
            return false;
        }
    }

    bool operator!=(const VarStruct &val)
    {
        switch (getType(val))
        {
        case VAR_TYPE_FLOAT:
            return vFloat != val.vFloat;
        case VAR_TYPE_LONG:
            return vInt != val.vInt;
        default:
            return false;
        }
    }

} VarStruct;

#endif