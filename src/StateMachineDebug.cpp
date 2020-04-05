#include "StateMachineDebug.h"

void silent_debug(const char *){};

DebugPrinter __debugPrinter = silent_debug;
