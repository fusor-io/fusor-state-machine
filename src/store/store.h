#ifndef store_h
#define store_h

#include <ArduinoJson.h>

#define MAX_VAR_NAME_LEN 32     // maximum length of variable name ("device-id.var-name.type")
#define MAX_VARIABLE_SPACE 1024 // maximum size of JSON storing local variables

class Store
{
public:
    Store(const char *);
    void attachGlobalMemory(JsonDocument *);

    void setVar(const char *, int);
    void setVar(const char *, float);

    int getVarInt(const char *, int defaultValue = 0);
    float getVarFloat(const char *, float defaultValue = 0.0f);
    JsonVariant getVar(const char *);

    char *nameWithScope(const char *);

private:
    DynamicJsonDocument _localMemory; // local device variables
    JsonDocument *_globalMemory;      // global variables populated from server

    const char *_deviceId;
    char _varNameBuffer[MAX_VAR_NAME_LEN];
};

#endif