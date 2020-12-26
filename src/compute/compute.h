#ifndef compute_h
#define compute_h

#include <ArduinoJson.h>

#include "../store/store.h"
#include "../timers/timers.h"
#include "../hooks/hooks.h"

#define M_UNKNOWN -1

#define M_UNARY 0
#define M_SQRT 1
#define M_EXP 2
#define M_LN 3
#define M_LOG 4
#define M_ABS 5
#define M_NEG 6

#define M_BINARY 100
#define M_SUB 101
#define M_DIV 102
#define M_POW 103

#define M_MULTI 200
#define M_SUM 201
#define M_MUL 202
#define M_MIN 203
#define M_MAX 204

#define C_UNKNOWN -1

#define C_BOOL 0
#define C_NOT 1
#define C_AND 2
#define C_OR 3

#define C_COMPARE 100
#define C_GT 101
#define C_GTE 102
#define C_LT 103
#define C_LTE 104
#define C_EQ 105
#define C_NE 106

#define C_SYSTEM 1000
#define C_ELAPSED 1001

class Compute
{
public:
    Compute(const char *, Timers *);

    Store store;

    bool evalCondition(JsonVariant);
    bool switchCondition(const char *, JsonVariant);
    float evalMath(JsonVariant);

    void setVar(const char *, float, bool isLocal = true);
    void setVar(const char *, long int, bool isLocal = true);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    long int getVarInt(const char *, long int defaultValue = 0);

    void setHooks(Hooks *hooks);

private:
    Timers *_timers;
    int _decodeMathOp(const char *);
    int _decodeConditionOp(const char *);
};

#endif