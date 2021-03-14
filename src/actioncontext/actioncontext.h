#ifndef actioncontext_h
#define actioncontext_h

class ActionContext; // forward ref

#include <ArduinoJson.h>
#include "../compute/compute.h"

class ActionContext
{
public:
    ActionContext(Compute *);

    size_t getCount();
    long int getParamInt(size_t, long int defaultValue = 0);
    float getParamFloat(size_t, float defaultValue = 0.0f);
    VarStruct getParam(size_t, long int defaultValue);
    VarStruct getParam(size_t, float defaultValue);

    void setParams(JsonArray *);
    void resetParams();

    Compute *compute;

private:
    JsonArray *_params;
};

#endif