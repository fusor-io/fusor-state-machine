#ifndef store_h
#define store_h

#include <map>
#include "math.h"
#include <ArduinoJson.h>
#include "../keycompare/keycompare.h"
#include "../keycreate/keycreate.h"
#include "../hooks/hooks.h"
#include "./varStruct.h"

#define MAX_VARIABLE_SPACE 1024 // maximum size of JSON storing local variables

class Store
{
public:
    Store(const char *);
    void setHooks(Hooks *);
    void attachGlobalMemory(JsonDocument *);

    void setVar(const char *, long int, bool isLocal = true);
    void setVar(const char *, int, bool isLocal = true);
    void setVar(const char *, float, bool isLocal = true);

    long int getVarInt(const char *, int defaultValue = 0);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    VarStruct *getVar(const char *);

    VarStruct *updateVar(VarStruct *, const char *, long int, bool onlyOnValueChange = true);
    VarStruct *updateVar(VarStruct *, const char *, int, bool onlyOnValueChange = true);
    VarStruct *updateVar(VarStruct *, const char *, float, bool onlyOnValueChange = true);

private:
    std::map<char *, VarStruct *, KeyCompare> _localMemory; // local device variables
    JsonDocument *_globalMemory;                            // global variables populated from server

    Hooks *_hooks = nullptr;
    const char *_deviceId;
    KeyCreate _keyCreator;
    char *_withScope(const char *);
};

#endif