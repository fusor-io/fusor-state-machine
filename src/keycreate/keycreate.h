#ifndef keycreate_h
#define keycreate_h

#define MAX_VAR_NAME_LEN 32 // maximum length of variable name ("device-id.plugin-id.var-name")

class KeyCreate
{
public:
    char *withScope(const char *, const char *);
    const char *createKey(const char *);

private:
    char _buffer[MAX_VAR_NAME_LEN];
};

#endif