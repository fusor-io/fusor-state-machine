#ifndef plugin_h
#define plugin_h

#include <map>

#include "../StateMachine.h"
#include "../keycompare/keycompare.h"

class Plugin;
typedef void (*PluginFunction)(Plugin *);

class Plugin
{
public:
    Plugin(const char *, StateMachineController *);

    StateMachineController *sm;
    const char *id;

    void registerAction(const char *, PluginFunction);
    void setVar(const char *, int);
    void setVar(const char *, float);
    int getVarInt(const char *, int defaultValue = 0);
    float getVarFloat(const char *, float defaultValue = 0.0f);

    std::map<const char *, PluginFunction, KeyCompare> actionMap;

private:
    char *withNameSpace(const char *);
};

#endif
