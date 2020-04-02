#include <string.h>

#include "../StateMachine.h"
#include "plugin.h"

Plugin::Plugin(const char *id, StateMachineController *sm) : actionMap()
{
    this->id = id;
    this->sm = sm;
}

void Plugin::registerAction(const char *name, PluginFunction action)
{
    actionMap[name] = action;
}

void Plugin::setVar(const char *name, int value)
{
    sm->compute.store.setVar(withNameSpace(name), value);
}

void Plugin::setVar(const char *name, float value)
{
    sm->compute.store.setVar(withNameSpace(name), value);
}

int Plugin::getVarInt(const char *name, int defaultValue)
{
    return sm->compute.store.getVarInt(withNameSpace(name), defaultValue);
}

float Plugin::getVarFloat(const char *name, float defaultValue)
{
    return sm->compute.store.getVarFloat(withNameSpace(name), defaultValue);
}

char *Plugin::withNameSpace(const char *name)
{
    char *buff = new char[strlen(name) + strlen(id) + 2];
    strcpy(buff, id);
    strcat(buff, ".");
    strcat(buff, name);
    return buff;
}