/*
  State Machine Controller -
  library for running configuration based state machines on Arduino
  Created by Giedrius Lukosevicius
  Released into the public domain
*/

#include <math.h>

#include "smcontroller.h"

#include "keycompare/keycompare.h"
#include "timers/timers.h"
#include "store/store.h"

std::map<const char *, ActionFunction, KeyCompare> StateMachineController::_actionMap;

/**************************************************************************
 *                         Initialization methods
 **************************************************************************/

StateMachineController::StateMachineController(const char *deviceId, SleepFunction sleepCallback, GetTimeFunction getTime)
    : timers(getTime), compute(deviceId, &timers)
{
  _deviceId = deviceId;
  _sleepCallback = sleepCallback;
}

void StateMachineController::registerAction(const char *name, ActionFunction func)
{
  _actionMap[name] = func;
}

void StateMachineController::setDefinition(JsonDocument *definition)
{
  _definition = definition;
}

void StateMachineController::init()
{
  _runInitAction();
  _initStateMachines();
}

/**************************************************************************
 *                             Main cycle
 **************************************************************************/

void StateMachineController::cycle()
{

  // Run actions berfore main loop

  if (_definition->containsKey(DEFINITION_BEFORE_ACTION))
  {
    _runActions((*_definition)[DEFINITION_BEFORE_ACTION]);
  }

  // Run main loop of state machines

  _runStateMachines();

  // Run actions after main loop

  if (_definition->containsKey(DEFINITION_AFTER_ACTION))
  {
    _runActions((*_definition)[DEFINITION_AFTER_ACTION]);
  }

  // Sleep for time specified in definition, or default 1000ms

  long timeout = 1000;

  if (_definition->containsKey(DEFINITION_SLEEP_TIMEOUT))
  {
    timeout = round(compute.evalMath((*_definition)[DEFINITION_SLEEP_TIMEOUT]));
  }

  if (timeout > 0)
    _sleep((unsigned long)timeout);
}

/**************************************************************************
 *                        Private methods
 **************************************************************************/

void StateMachineController::_yield()
{
  _sleep(0);
}

void StateMachineController::_sleep(unsigned long ms)
{
  if (_sleepCallback)
    _sleepCallback(ms);
}

void StateMachineController::_runAction(JsonVariant action)
{
  // check if action is string and it is not empty
  if (action.is<char *>() && ((const char *)action)[0])
  {
    if (_actionMap.count((const char *)action))
    {
      _actionMap[(const char *)action](this);
    }
  }
}

void StateMachineController::_runActions(JsonVariant actions)
{
  if (!actions.isNull() && actions.is<JsonArray>())
  {
    for (JsonVariant action : (JsonArray)actions)
    {
      _runAction(action);
      _yield();
    }
  }
}

void StateMachineController::_runInitAction()
{
  _runActions((*_definition)[DEFINITION_INIT_ACTION]);
}

void StateMachineController::_initStateMachines()
{
  auto state_machines = (*_definition)[DEFINITION_STATE_MACHINES];

  if (!state_machines.is<JsonObject>())
    return;

  _stateMachineCount = 0;
  for (JsonPair state_machine : (JsonObject)state_machines)
  {

    // validate machine

    JsonVariant item = state_machine.value();
    if (!item.is<JsonObject>())
      continue;

    JsonObject machine = item.as<JsonObject>();
    JsonVariant states_definition = machine[SM_STATES];

    if (!states_definition.is<JsonObject>() && !states_definition.isNull())
      continue;

    // load machine

    STATE_MACHINE_SLOT *slot = &_stateMachines[_stateMachineCount];

    slot->name = state_machine.key().c_str();
    slot->machine = machine;
    slot->states_definition = states_definition.as<JsonObject>();

    // run initial actions

    _runActions(machine[SM_INITIAL_ACTIONS]);

    // set machine to the starting state

    auto initial_state = machine[SM_INITIAL_STATE];
    if (initial_state.is<char *>() && ((const char *)initial_state)[0])
    {
      _switchState(slot, (const char *)initial_state);
    }
    else
    {
      // no initial state => state machine will not be working
      slot->state = nullptr;
    }

    if (++_stateMachineCount >= MAX_STATE_MACHINES)
    {
      --_stateMachineCount;
      break;
    }
  }
}

void StateMachineController::_switchState(STATE_MACHINE_SLOT *machineDefinition, const char *newState)
{
  if (!newState[0])
    return;

  // set new state
  machineDefinition->state = newState;

  // run initial state actions
  JsonVariant state_definition = machineDefinition->states_definition[newState];

  if (!state_definition.is<JsonObject>())
    return;
  JsonVariant entryActions = state_definition[STATE_ENTRY_ACTIONS];
  _runActions(entryActions);
}

void StateMachineController::_runStateMachines()
{
  for (int i = 0; i < _stateMachineCount; i++)
  {

    // take machine

    const char *state = _stateMachines[i].state;
    JsonObject states_definition = _stateMachines[i].states_definition;

    // check if machine has defined states

    if (states_definition.isNull())
      continue;

    // get state definition

    JsonVariant state_definition = states_definition[state];
    if (!state_definition.is<JsonObject>() || state_definition.isNull())
      continue;

    // get rules

    JsonVariant rules = state_definition.as<JsonObject>()[STATE_EXIT_RULES];
    if (!rules.is<JsonArray>() || rules.isNull())
      continue;

    // check if any rule can be applied to get the next state

    const char *nextState = _getNextState(rules);
    _yield();

    if (nextState == nullptr)
      continue;

    // swith state
    _switchState(&_stateMachines[i], nextState);
  }
}

const char *StateMachineController::_getNextState(JsonArray rules)
{
  for (JsonVariant item : rules)
  {
    // validate rule

    if (!item.is<JsonObject>() || item.isNull())
      continue;

    JsonObject rule = item.as<JsonObject>();
    if (rule[STATE_RULE_IF].isNull() ||
        rule[STATE_RULE_THEN].isNull() ||
        !rule[STATE_RULE_THEN].is<char *>() ||
        !rule[STATE_RULE_THEN].as<char *>()[0])
      continue;

    // check rule

    JsonVariant condition = rule[STATE_RULE_IF];
    const char *target_state = rule[STATE_RULE_THEN].as<char *>();

    if (compute.evalCondition(rule[STATE_RULE_IF]))
    {
      // rule satisfied, return next state name
      return target_state;
    }
  }

  return nullptr;
}