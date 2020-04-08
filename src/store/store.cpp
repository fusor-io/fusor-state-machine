#include <math.h>
#include <ArduinoJson.h>

#include "store.h"
#include "../StateMachineDebug.h"
#include "../keycreate/keycreate.h"

Store::Store(const char *deviceId)
    : _localMemory(), _keyCreator()
{
    _deviceId = deviceId;
    _globalMemory = nullptr;
}

void Store::attachGlobalMemory(JsonDocument *memory)
{
    _globalMemory = memory;
}

void Store::setVar(const char *varName, long int value)
{
    char *varNameWithScope = _withScope(varName);

    // we can set only local variables. So need to add scope id
    SM_DEBUG("Set int var [" << varNameWithScope << "]: " << value << "\n");
    if (_localMemory.count(varNameWithScope))
    {
        VarStruct *var = _localMemory[varNameWithScope];
        var->type = VAR_TYPE_LONG;
        var->vInt = value;
        var->vFloat = (float)value;
    }
    else
    {
        _localMemory[(char *)_keyCreator.createKey(varNameWithScope)] = new VarStruct(value);
    }
}

void Store::setVar(const char *var_name, int value)
{
    setVar(var_name, (long int)value);
}

void Store::setVar(const char *varName, float value)
{
    char *varNameWithScope = _withScope(varName);
    // we can set only local variables. So need to add scope id
    SM_DEBUG("Set float var [" << varNameWithScope << "]: " << value << "\n");
    if (_localMemory.count(varNameWithScope) > 0)
    {
        VarStruct *var = _localMemory[varNameWithScope];
        var->type = VAR_TYPE_FLOAT;
        var->vInt = round(value);
        var->vFloat = value;
    }
    else
    {
        _localMemory[(char *)_keyCreator.createKey(varNameWithScope)] = new VarStruct(value);
    }
}

int Store::getVarInt(const char *name, int defaultValue)
{
    VarStruct *value = getVar(name);

    if (value == nullptr)
        return defaultValue;

    return value->vInt;
}

float Store::getVarFloat(const char *name, float defaultValue)
{
    VarStruct *value = getVar(name);

    if (value == nullptr)
        return defaultValue;

    return value->vFloat;
}

VarStruct *Store::getVar(const char *varName)
{
    // first check in local variables

    SM_DEBUG("Get var: " << varName << "\n");

    if (_localMemory.count((char *)varName) > 0)
    {
        SM_DEBUG("Get var [" << varName << "] = " << _localMemory[(char *)varName]->vFloat << "\n");
        return _localMemory[(char *)varName];
    }

    SM_DEBUG("Var " << varName << " not found\n");

    // var_name can be in a local format (no scope identifier)
    // so try adding devideId as a scope

    char *varNameWithScope = _withScope(varName);
    if (_localMemory.count(varNameWithScope) > 0)
    {
        SM_DEBUG("Get var [" << varNameWithScope << "] = " << _localMemory[varNameWithScope]->vFloat << "\n");
        return _localMemory[varNameWithScope];
    }

    SM_DEBUG("Var " << varNameWithScope << " not found\n");

    // last chance is that it is external variable
    // if (_globalMemory)
    //     return (*_globalMemory)[varName];

    return nullptr;
}

char *Store::_withScope(const char *var_name)
{
    return _keyCreator.withScope(_deviceId, var_name);
}
