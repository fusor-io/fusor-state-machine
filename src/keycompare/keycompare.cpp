#include "keycompare.h"
#include <string.h>

bool KeyCompare::operator()(char const *a, char const *b) const
{
    return strcasecmp(a, b) < 0;
}
