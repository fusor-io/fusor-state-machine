#include <math.h>
#include <ArduinoJson.h>
#include "store.h"

Store::Store(const char *deviceId)
    : _localMemory(MAX_VARIABLE_SPACE)
{
    _deviceId = deviceId;
    _globalMemory = nullptr;
}

void Store::attachGlobalMemory(JsonDocument *memory)
{
    _globalMemory = memory;
}

void Store::setVar(const char *var_name, int value)
{
    // we can set only local variables. So need to add scope id
    _localMemory[nameWithScope(var_name)] = value;
}

void Store::setVar(const char *var_name, float value)
{
    // we can set only local variables. So need to add scope id
    _localMemory[nameWithScope(var_name)] = value;
}

int Store::getVarInt(const char *name, int defaultValue)
{
    JsonVariant value = getVar(name);

    if (value.is<int>())
        return value.as<int>();

    if (value.is<float>())
        return round(value.as<float>());

    return defaultValue;
}

float Store::getVarFloat(const char *name, float defaultValue)
{
    JsonVariant value = getVar(name);

    if (value.isNull())
        return defaultValue;

    if (value.is<float>())
        return value.as<float>();

    if (value.is<int>())
        return (float)value.as<int>();

    return defaultValue;
}

JsonVariant Store::getVar(const char *var_name)
{

    // first check in local variables

    JsonVariant value = _localMemory[var_name];
    if (!value.isNull())
        return value;

    // var_name can be in a local format (no scope identifier)
    // so try adding devideId as a scope

    value = _localMemory[nameWithScope(var_name)];
    if (!value.isNull())
        return value;

    // last chance is that it is external variable
    if (_globalMemory)
        return (*_globalMemory)[var_name];

    return value;
}

char *Store::nameWithScope(const char *var_name)
{
    strncpy(_varNameBuffer, _deviceId, MAX_VAR_NAME_LEN - 1);
    strncat(_varNameBuffer, ".", MAX_VAR_NAME_LEN - strlen(_varNameBuffer) - 1);
    strncat(_varNameBuffer, var_name, MAX_VAR_NAME_LEN - strlen(_varNameBuffer) - 1);
    return _varNameBuffer;
}
