#include "actioncontext.h"

ActionContext::ActionContext(Compute *compute)
{
    this->compute = compute;
}

size_t ActionContext::getCount()
{
    return _params == nullptr ? 0 : _params->size();
}

long int ActionContext::getParamInt(size_t paramPosition, long int defaultValue)
{
    return getCount() > paramPosition ? compute->evalMath(_params->getElement(paramPosition)).vInt : defaultValue;
}

float ActionContext::getParamFloat(size_t paramPosition, float defaultValue)
{
    return getCount() > paramPosition ? compute->evalMath(_params->getElement(paramPosition)).vFloat : defaultValue;
}

VarStruct ActionContext::getParam(size_t paramPosition, long int defaultValue)
{
    return getCount() > paramPosition ? compute->evalMath(_params->getElement(paramPosition)) : defaultValue;
}

VarStruct ActionContext::getParam(size_t paramPosition, float defaultValue)
{
    return getCount() > paramPosition ? compute->evalMath(_params->getElement(paramPosition)) : defaultValue;
}

void ActionContext::setParams(JsonArray *params)
{
    _params = params;
}

void ActionContext::resetParams()
{
    _params = nullptr;
}
