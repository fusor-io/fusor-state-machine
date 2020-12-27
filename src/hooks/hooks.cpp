#include "hooks.h"
#include <Arduino.h>

void Hooks::onVarUpdate(const char *name, VarStruct *value) {}
void Hooks::afterCycle(unsigned long cycleNum) {}