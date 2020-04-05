#ifndef statemachinedebug_h
#define statemachinedebug_h

typedef void (*DebugPrinter)(const char *);

#ifdef SM_DEBUGGER

#include <stdio.h>
#include <ArduinoJson.h>

#define DEBUG_BUFFER_SIZE 2048

extern DebugPrinter __debugPrinter;

class Trace
{
public:
    char buff[DEBUG_BUFFER_SIZE];
    inline static Trace &GetTrace()
    {
        static Trace trace;
        return trace;
    }
    inline Trace &operator<<(const char *value)
    {
        __debugPrinter(value);
        return *this;
    };
    inline Trace &operator<<(char *value)
    {
        __debugPrinter(value);
        return *this;
    };
    inline Trace &operator<<(int value)
    {
        sprintf(buff, "%d", value);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(long int value)
    {
        sprintf(buff, "%ld", value);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(unsigned long value)
    {
        sprintf(buff, "%lu", value);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(float value)
    {
        // on Arduion "%f" does not work
        // so we produce fixed #.#### format manually

        long int fixed = (long int)(value * 10000.0 + 0.5);
        sprintf(buff, "%ld.%04ld", fixed / 10000L, fixed % 10000L);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(const JsonDocument &doc)
    {
        serializeJson(doc, buff, DEBUG_BUFFER_SIZE - 1);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(const JsonVariant &doc)
    {
        serializeJson(doc, buff, DEBUG_BUFFER_SIZE - 1);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(const JsonObject &doc)
    {
        serializeJson(doc, buff, DEBUG_BUFFER_SIZE - 1);
        __debugPrinter(buff);
        return *this;
    }
    inline Trace &operator<<(Trace &(*function)(Trace &trace))
    {
        return function(*this);
    }
};

#define SM_DEBUG(output) Trace::GetTrace() << output
#else
#define SM_DEBUG(output)
#endif
#endif
