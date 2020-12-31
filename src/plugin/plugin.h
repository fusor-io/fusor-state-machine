#ifndef plugin_h
#define plugin_h

#include <map>

#include "../StateMachine.h"
#include "../keycompare/keycompare.h"
#include "../actioncontext/actioncontext.h"

class Plugin;
typedef void (*PluginFunction)(Plugin *);

class Plugin
{
public:
    Plugin(const char *);

    StateMachineController *sm;
    const char *id;

    virtual void initialize(StateMachineController *);
    void registerAction(const char *, PluginFunction);
    void setVar(const char *, long int);
    void setVar(const char *, int);
    void setVar(const char *, float);
    int getVarInt(const char *, int defaultValue = 0);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    VarStruct *getVarRaw(const char *);

    unsigned long getElapsedTime(unsigned long);

    std::map<const char *, PluginFunction, KeyCompare> actionMap;

    ActionContext *actionContext;

protected:
    KeyCreate _keyCreator;
    char *_withScope(const char *);
};

#endif
