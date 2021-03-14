#include <ArduinoJson.h>

#include <math.h>
#include <float.h>

#include "compute.h"
#include "../timers/timers.h"

#include "../StateMachineDebug.h"

Compute::Compute(const char *deviceId, Timers *timers)
    : store(deviceId), _mathFunctionMap(), _boolFunctionMap()
{
    _timers = timers;
}

void Compute::registerFunction(const char *name, MathFunction func)
{
    _mathFunctionMap[name] = func;
}

void Compute::registerFunction(const char *name, BoolFunction func)
{
    _boolFunctionMap[name] = func;
}

void Compute::setVar(const char *varName, const VarStruct &value, bool isLocal)
{
    store.setVar(varName, value, isLocal);
}

void Compute::setVar(const char *varName, float value, bool isLocal)
{
    store.setVar(varName, value, isLocal);
}

void Compute::setVar(const char *varName, long int value, bool isLocal)
{
    store.setVar(varName, value, isLocal);
}

float Compute::getVarFloat(const char *varName, float defaultValue)
{
    return store.getVarFloat(varName, defaultValue);
}

long int Compute::getVarInt(const char *varName, long int defaultValue)
{
    return store.getVarInt(varName, defaultValue);
}

void Compute::setHooks(Hooks *hooks)
{
    store.setHooks(hooks);
}

bool Compute::evalCondition(JsonVariant condition)
{

    // try interpreting primitive values

    if (condition.isNull())
        return false;
    if (condition.is<bool>())
        return (bool)condition.as<bool>();
    if (condition.is<int>())
        return (bool)condition.as<int>();
    if (condition.is<float>())
        return condition.as<float>() != 0.0;
    if (condition.is<char *>())
    {
        return ((char *)condition.as<char *>())[0] ? store.getVarInt(condition) : false;
    }
    if (!condition.is<JsonObject>())
        return false;

    // we've got object here, so let's get the first property
    // it should be the only property, so we are fine just taking the first one

    JsonObject::iterator condition_object = ((JsonObject)condition.as<JsonObject>()).begin();

    // check if property actually has name

    const char *operation = condition_object->key().c_str();
    if (!operation[0])
        return false;

    return switchCondition(operation, condition_object->value());
}

bool Compute::switchCondition(const char *operation, JsonVariant operands)
{

    // lowercase the operation name

    int op = _decodeConditionOp(operation);

    // check if this is the only unary operation

    if (op == C_NOT)
    {
        if (operands.is<JsonArray>())
        {
            JsonArray arr = operands.as<JsonArray>();
            if (arr.size() < 1)
                return false;
            return !evalCondition(arr[0]);
        }
        else
        {
            return !evalCondition(operands);
        }
    }

    // non unary operations requires array of operands, check that

    if (!operands.is<JsonArray>())
        return false;
    JsonArray arr = operands.as<JsonArray>();

    // evaluate operations

    if (op == C_AND)
    {

        bool res = true;
        for (JsonVariant operand : arr)
        {
            res = res && evalCondition(operand);
            if (!res)
                return false;
        }
        return res;
    }
    else if (op == C_OR)
    {

        bool res = false;
        for (JsonVariant operand : arr)
        {
            res = res || evalCondition(operand);
            if (res)
                return true;
        }
        return res;
    }
    else if (op > C_COMPARE && op < C_SYSTEM)
    {

        if (operands.size() < 2)
            return false;
        VarStruct op1 = evalMath(arr[0]);
        VarStruct op2 = evalMath(arr[1]);

        switch (op)
        {
        case C_GT:
            return op1 > op2;
        case C_GTE:
            return op1 >= op2;
        case C_LT:
            return op1 < op2;
        case C_LTE:
            return op1 <= op2;
        case C_EQ:
            return op1 == op2;
        case C_NE:
            return op1 != op2;
        }
    }
    else if (op == C_ELAPSED)
    {
        if (operands.size() < 2 || !operands[0].is<const char *>() || _timers == nullptr)
            return true;

        const char *timerName = operands[0].as<const char *>();
        if (!timerName[0])
            return true;

        unsigned long timeout = evalMath(operands[1]).vInt;

        return _timers->validateTimer(timerName, timeout);
    }

    if (_boolFunctionMap.count(operation))
        return _execBoolFunction(operation, operands);

    return false;
}

VarStruct Compute::evalMath(JsonVariant object)
{

    SM_DEBUG("Eval math: " << object << "\n");

    if (object.isNull())
        return 0l;
    if (object.is<bool>())
        return VarStruct((long int)object.as<bool>());
    if (object.is<int>())
        return VarStruct((long int)object.as<int>());
    if (object.is<float>())
        return VarStruct(object.as<float>());
    if (object.is<char *>())
    {
        // if type is string, we should look for variable of that name
        const char *varName = (char *)object.as<char *>();
        SM_DEBUG("Operand " << varName << " is a string. Evaluating it as var\n");
        VarStruct *var = store.getVar(varName);
        return var == nullptr ? VarStruct(0l) : VarStruct(*var);
    }
    if (object.is<JsonArray>())
    {
        // we shouldn't be here
        // but if we are, just eval first element
        JsonArray arr = object.as<JsonArray>();
        if (arr.size() < 1)
            return 0l;
        return evalMath(arr[0]);
    }

    if (!object.is<JsonObject>())
        return 0l; // what was that?

    // it should be the only property, so let's get the first one

    JsonObject obj = object.as<JsonObject>();
    JsonObject::iterator operation_iter = obj.begin();

    // check if object is empty
    if (!obj.size())
        return 0l;

    // check if property actually has name
    const char *operation = operation_iter->key().c_str();
    if (!operation[0])
        return 0l;
    JsonVariant operands = operation_iter->value();

    int op = _decodeMathOp(operation);

    if (op > M_NULLARY && op < M_UNARY)
    {
        switch (op)
        {
        case M_TICKS:
            return (long int)_timers->getTime();
        default:
            return 0l;
        }
    }
    else if (op > M_UNARY && op < M_BINARY)
    {

        // unary operations

        VarStruct operand = evalMath(operands);

        switch (op)
        {

        case M_SQRT:

            if (operand < 0l)
                return VarStruct::NaN();
            return (float)sqrt(operand.vFloat);

        case M_EXP:

            return (float)exp(operand.vFloat);

        case M_LN:

            if (operand <= 0l)
                return VarStruct::NaN();
            return (float)log(operand.vFloat);

        case M_LOG:

            if (operand <= 0l)
                return VarStruct::NaN();
            return (float)log10(operand.vFloat);

        case M_ABS:

            return operand >= 0l ? operand : -operand;

        case M_NEG:
        default:

            return -operand;
        }
    }
    else if (op > M_BINARY && op < M_TRINARY)
    {
        // binary operations

        // lets do preflight check first

        if (!operands.is<JsonArray>())
        {
            // we shouldn't be here, but if we are, just return value
            return evalMath(operands);
        }

        JsonArray arr = operands.as<JsonArray>();

        if (arr.size() == 0)
            return 0l;
        if (arr.size() == 1)
        {
            if (op == M_SUB)
                return -evalMath(operands[0]);
            return evalMath(operands[0]);
        }

        switch (op)
        {

        case M_SUB:

            return evalMath(operands[0]) - evalMath(operands[1]);

        case M_DIV:

            return evalMath(operands[0]) / evalMath(operands[1]);

        case M_POW:

            return (float)pow(evalMath(operands[0]).vFloat, evalMath(operands[1]).vFloat);

        case M_DIFF:
        default:

            unsigned long a = (unsigned long)evalMath(operands[0]).vInt;
            unsigned long b = (unsigned long)evalMath(operands[1]).vInt;
            return (long int)std::min(_timers->diff(a, b), _timers->diff(b, a));
        }
    }
    else if (op > M_TRINARY && op < M_MULTI)
    {
        // trinary operations

        // lets do preflight check first

        if (!operands.is<JsonArray>())
        {
            // we shouldn't be here, but if we are, just return value
            return evalMath(operands);
        }

        JsonArray arr = operands.as<JsonArray>();

        size_t size = arr.size();
        if (size == 0)
            return 0l;

        switch (op)
        {
        case M_IF:
        default:
            if (size == 1)
                return 0l;
            if (size == 2)
                return evalCondition(arr[0]) ? evalMath(arr[1]) : 0l;
            return evalMath(evalCondition(arr[0]) ? arr[1] : arr[2]);
        }
    }
    else if (op > M_MULTI)
    {

        // multi operand operations

        JsonArray arr = operands.as<JsonArray>();
        if (arr.size() == 0)
            return 0l;

        VarStruct res;

        switch (op)
        {

        case M_SUM:

            res = 0l;
            for (JsonVariant operand : arr)
                res = res + evalMath(operand);
            return res;

        case M_MUL:

            res = 1l;
            for (JsonVariant operand : arr)
                res = res * evalMath(operand);
            return res;

        case M_MIN:
        {

            unsigned cnt = 0;
            for (JsonVariant operand : arr)
            {
                VarStruct val = evalMath(operand);
                if (cnt++ == 0 || res > val)
                    res = val;
            }
            return res;
        }

        case M_MAX:
        default:
        {
            unsigned cnt = 0;
            for (JsonVariant operand : arr)
            {
                VarStruct val = evalMath(operand);
                if (cnt++ == 0 || res < val)
                    res = val;
            }
            return res;
        }
        }
    }

    if (_mathFunctionMap.count(operation))
        return _execMathFunction(operation, operands);

    return 0l;
}

int Compute::_decodeMathOp(const char *op)
{
    if (strcasecmp(op, "sqrt") == 0)
        return M_SQRT;
    if (strcasecmp(op, "exp") == 0)
        return M_EXP;
    if (strcasecmp(op, "ln") == 0)
        return M_LN;
    if (strcasecmp(op, "log") == 0)
        return M_LOG;
    if (strcasecmp(op, "abs") == 0)
        return M_ABS;
    if (strcasecmp(op, "neg") == 0)
        return M_NEG;
    if (strcasecmp(op, "sub") == 0)
        return M_SUB;
    if (strcasecmp(op, "div") == 0)
        return M_DIV;
    if (strcasecmp(op, "pow") == 0)
        return M_POW;
    if (strcasecmp(op, "sum") == 0)
        return M_SUM;
    if (strcasecmp(op, "mul") == 0)
        return M_MUL;
    if (strcasecmp(op, "min") == 0)
        return M_MIN;
    if (strcasecmp(op, "max") == 0)
        return M_MAX;
    if (strcasecmp(op, "?") == 0)
        return M_IF;
    if (strcasecmp(op, "ticks") == 0) // current time in OS units (provided by _getTimeCallback)
        return M_TICKS;
    if (strcasecmp(op, "diff") == 0) // time difference in OS units, for short periods (timer overflow safe)
        return M_DIFF;

    return M_UNKNOWN;
}

int Compute::_decodeConditionOp(const char *op)
{
    if (strcasecmp(op, "not") == 0)
        return C_NOT;
    if (strcasecmp(op, "and") == 0)
        return C_AND;
    if (strcasecmp(op, "or") == 0)
        return C_OR;
    if (strcasecmp(op, "gt") == 0)
        return C_GT;
    if (strcasecmp(op, "gte") == 0)
        return C_GTE;
    if (strcasecmp(op, "lt") == 0)
        return C_LT;
    if (strcasecmp(op, "lte") == 0)
        return C_LTE;
    if (strcasecmp(op, "eq") == 0)
        return C_EQ;
    if (strcasecmp(op, "ne") == 0)
        return C_NE;
    if (strcasecmp(op, "elapsed") == 0)
        return C_ELAPSED;

    return C_UNKNOWN;
}

VarStruct Compute::_execMathFunction(const char *name, JsonVariant params)
{
    ActionContext context(this);
    JsonArray arr;

    if (!params.isNull() && params.is<JsonArray>())
    {
        arr = params.as<JsonArray>();
        context.setParams(&arr);
    }

    return _mathFunctionMap[name](&context);
}

bool Compute::_execBoolFunction(const char *name, JsonVariant params)
{
    ActionContext context(this);
    JsonArray arr;

    if (!params.isNull() && params.is<JsonArray>())
    {
        arr = params.as<JsonArray>();
        context.setParams(&arr);
    }

    return _boolFunctionMap[name](&context);
}
