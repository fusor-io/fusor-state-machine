#ifndef store_h
#define store_h

#include <map>
#include "math.h"
#include <ArduinoJson.h>
#include "../keycompare/keycompare.h"

#define MAX_VAR_NAME_LEN 32     // maximum length of variable name ("device-id.var-name.type")
#define MAX_VARIABLE_SPACE 1024 // maximum size of JSON storing local variables

#define VAR_TYPE_FLOAT 0
#define VAR_TYPE_LONG 1

typedef struct VarStruct
{
    VarStruct(long int _value)
        : type(VAR_TYPE_LONG), vFloat((float)_value), vInt(_value) {}
    VarStruct(float _value)
        : type(VAR_TYPE_LONG), vFloat(_value), vInt(round(_value)) {}

    char type;
    volatile long int vInt;
    volatile float vFloat;
} VarStruct;

class Store
{
public:
    Store(const char *);
    void attachGlobalMemory(JsonDocument *);

    void setVar(const char *, long int);
    void setVar(const char *, int);
    void setVar(const char *, float);

    int getVarInt(const char *, int defaultValue = 0);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    VarStruct *getVar(const char *);

    char *nameWithScope(const char *);

private:
    std::map<char *, VarStruct *, KeyCompare> _localMemory; // local device variables
    JsonDocument *_globalMemory;                            // global variables populated from server

    const char *_deviceId;
    char _varNameBuffer[MAX_VAR_NAME_LEN];
    char *_createKey(const char *);
};

#endif