#include <string.h>

#include "../StateMachine.h"
#include "../keycreate/keycreate.h"
#include "plugin.h"

Plugin::Plugin(const char *id) : actionMap(), _keyCreator()
{
    this->id = id;
}

void Plugin::initialize(StateMachineController *sm)
{
    this->sm = sm;
}

void Plugin::registerAction(const char *name, PluginFunction action)
{
    actionMap[name] = action;
}

void Plugin::setVar(const char *name, int value)
{
    sm->compute.store.setVar(_withScope(name), value);
}

void Plugin::setVar(const char *name, float value)
{
    sm->compute.store.setVar(_withScope(name), value);
}

int Plugin::getVarInt(const char *name, int defaultValue)
{
    return sm->compute.store.getVarInt(_withScope(name), defaultValue);
}

float Plugin::getVarFloat(const char *name, float defaultValue)
{
    return sm->compute.store.getVarFloat(_withScope(name), defaultValue);
}

VarStruct *Plugin::getVarRaw(const char *name)
{
    return sm->compute.store.getVar(_withScope(name));
}

char *Plugin::_withScope(const char *varName)
{
    return _keyCreator.withScope(id, varName);
}
