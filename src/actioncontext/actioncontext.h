#ifndef actioncontext_h
#define actioncontext_h

#include <ArduinoJson.h>
#include "../compute/compute.h"

class ActionContext
{
public:
    ActionContext(Compute *);

    size_t getCount();
    long int getParamInt(size_t, long int defaultValue = 0);
    float getParamFloat(size_t, float defaultValue = 0.0f);

    void setParams(JsonArray *);
    void resetParams();

    Compute *compute;

private:
    JsonArray *_params;
};

#endif