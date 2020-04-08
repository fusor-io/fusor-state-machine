#include <string.h>
#include "keycreate.h"

char *KeyCreate::withScope(const char *scopeId, const char *name)
{
    strncpy(_buffer, scopeId, MAX_VAR_NAME_LEN - 1);
    strncat(_buffer, ".", MAX_VAR_NAME_LEN - strlen(_buffer) - 1);
    strncat(_buffer, name, MAX_VAR_NAME_LEN - strlen(_buffer) - 1);
    return _buffer;
}

const char *KeyCreate::createKey(const char *name)
{
    char *buff = new char[strlen(name) + 1];
    strcpy(buff, name);
    return buff;
}
