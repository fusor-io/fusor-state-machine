#include <ArduinoJson.h>

#include <math.h>
#include <float.h>

#include "compute.h"
#include "../timers/timers.h"

#include "../StateMachineDebug.h"

Compute::Compute(const char *deviceId, Timers *timers)
    : store(deviceId)
{
    _timers = timers;
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
        float op1 = evalMath(arr[0]);
        float op2 = evalMath(arr[1]);

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

        unsigned long timeout = round(evalMath(operands[1]));

        return _timers->validateTimer(timerName, timeout);
    }

    return false;
}

float Compute::evalMath(JsonVariant object)
{

    SM_DEBUG("Eval math: " << object << "\n");

    if (object.isNull())
        return 0.0;
    if (object.is<bool>())
        return (float)object.as<bool>();
    if (object.is<int>())
        return (float)object.as<int>();
    if (object.is<float>())
        return object.as<float>();
    if (object.is<char *>())
    {
        // if type is string, we should look for variable of that name
        const char *varName = (char *)object.as<char *>();
        SM_DEBUG("Operand " << varName << " is a string. Evaluating it as var\n");
        return varName[0] ? store.getVarFloat(varName) : 0.0;
    }
    if (object.is<JsonArray>())
    {
        // we shouldn't be here
        // but if we are, just eval first element
        JsonArray arr = object.as<JsonArray>();
        if (arr.size() < 1)
            return 0.0;
        return evalMath(arr[0]);
    }

    if (!object.is<JsonObject>())
        return 0.0; // what was that?

    // it should be the only property, so let's get the first one

    JsonObject::iterator operation_iter = ((JsonObject)object.as<JsonObject>()).begin();

    // check if property actually has name
    const char *operation = operation_iter->key().c_str();
    if (!operation[0])
        return 0.0;
    JsonVariant operands = operation_iter->value();

    int op = _decodeMathOp(operation);

    if (op > M_UNARY && op < M_BINARY)
    {

        // unary operations

        float operand = evalMath(operands);

        switch (op)
        {

        case M_SQRT:

            // not correct, but we dont want throw
            // TODO: implement error handling

            if (operand < 0.0)
                return FLT_MIN;
            return sqrt(operand);

        case M_EXP:

            return exp(operand);

        case M_LN:

            // not correct, but we dont want throw
            // TODO: implement NaN

            if (operand <= 0.0)
                return FLT_MIN;
            return log(operand);

        case M_LOG:

            // not correct, but we dont want throw
            // TODO: implement error handling

            if (operand <= 0.0)
                return FLT_MIN;
            return log10(operand);

        case M_ABS:

            return abs(operand);

        case M_NEG:
        default:

            return -operand;
        }
    }
    else if (op > M_BINARY && op < M_MULTI)
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
            return 0.0;
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
        default:

            return pow(evalMath(operands[0]), evalMath(operands[1]));
        }
    }
    else if (op > M_MULTI)
    {

        // multi operand operations

        JsonArray arr = operands.as<JsonArray>();
        if (arr.size() == 0)
            return 0.0;

        float res;

        switch (op)
        {

        case M_SUM:

            res = 0.0;
            for (JsonVariant operand : arr)
                res += evalMath(operand);
            return res;

        case M_MUL:

            res = 1.0;
            for (JsonVariant operand : arr)
                res *= evalMath(operand);
            return res;

        case M_MIN:

            res = FLT_MAX;
            for (JsonVariant operand : arr)
                res = std::min(res, evalMath(operand));
            return res;

        case M_MAX:
        default:

            res = FLT_MIN;
            for (JsonVariant operand : arr)
                res = std::max(res, evalMath(operand));
            return res;
        }
    }

    return 0.0;
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