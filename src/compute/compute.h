#ifndef compute_h
#define compute_h

class Compute; // forward ref

#include <map>

#include <ArduinoJson.h>

#include "../store/store.h"
#include "../timers/timers.h"
#include "../hooks/hooks.h"
#include "../actioncontext/actioncontext.h"
#include "../keycompare/keycompare.h"

#define M_UNKNOWN -1

#define M_NULLARY 0
#define M_TICKS 1

#define M_UNARY 100
#define M_SQRT 101
#define M_EXP 102
#define M_LN 103
#define M_LOG 104
#define M_ABS 105
#define M_NEG 106

#define M_BINARY 200
#define M_SUB 201
#define M_DIV 202
#define M_POW 203
#define M_DIFF 204

#define M_TRINARY 300
#define M_IF 301

#define M_MULTI 400
#define M_SUM 401
#define M_MUL 402
#define M_MIN 403
#define M_MAX 404

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

typedef VarStruct (*MathFunction)(ActionContext *);
typedef bool (*BoolFunction)(ActionContext *);

class Compute
{
public:
    Compute(const char *, Timers *);

    Store store;

    void registerFunction(const char *, MathFunction);
    void registerFunction(const char *, BoolFunction);

    bool evalCondition(JsonVariant);
    bool switchCondition(const char *, JsonVariant);
    VarStruct evalMath(JsonVariant);

    void setVar(const char *, const VarStruct &, bool isLocal = true);
    void setVar(const char *, float, bool isLocal = true);
    void setVar(const char *, long int, bool isLocal = true);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    long int getVarInt(const char *, long int defaultValue = 0);

    void setHooks(Hooks *hooks);

private:
    Timers *_timers;
    int _decodeMathOp(const char *);
    int _decodeConditionOp(const char *);

    std::map<const char *, MathFunction, KeyCompare> _mathFunctionMap;
    std::map<const char *, BoolFunction, KeyCompare> _boolFunctionMap;

    VarStruct _execMathFunction(const char *, JsonVariant);
    bool _execBoolFunction(const char *, JsonVariant);
};

#endif