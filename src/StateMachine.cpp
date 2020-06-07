/*
  State Machine Controller -
  library for running configuration based state machines on Arduino
  Copyright Giedrius Lukosevicius 2020
  MIT License
*/

// Uncomment the following line and recompile library
// to enable debugg printing
// #define SM_DEBUGGER

#include <math.h>

#include "StateMachine.h"

#include "keycompare/keycompare.h"
#include "keycreate/keycreate.h"
#include "timers/timers.h"
#include "store/store.h"
#include "actioncontext/actioncontext.h"

#include "StateMachineDebug.h"

/**************************************************************************
 *                         Initialization methods
 **************************************************************************/

StateMachineController::StateMachineController(const char *deviceId, SleepFunction sleepCallback, GetTimeFunction getTime)
    : timers(getTime), compute(deviceId, &timers), _actionMap(), _pluginMap(), _actionContext(&compute)
{
  _deviceId = deviceId;
  _sleepCallback = sleepCallback;
}

void StateMachineController::registerAction(const char *name, ActionFunction func)
{
  _actionMap[name] = func;
}

void StateMachineController::registerPlugin(Plugin *plugin)
{
  _pluginMap[plugin->id] = plugin;
  plugin->initialize(this);
}

void StateMachineController::setDefinition(JsonDocument *definition)
{
  SM_DEBUG("State Machine definition: " << *definition << "\n");
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

  SM_DEBUG("==============================================================\n");
  SM_DEBUG("Entering cycle\n");

  // Run actions berfore main loop

  if (_definition->containsKey(DEFINITION_BEFORE_ACTION))
  {
    _runActions((*_definition)[DEFINITION_BEFORE_ACTION]);
  }

  // Run main loop of state machines

  _runStateMachines();

  // Run actions after main loop

  SM_DEBUG("Checking post loop actions\n");

  if (_definition->containsKey(DEFINITION_AFTER_ACTION))
  {
    _runActions((*_definition)[DEFINITION_AFTER_ACTION]);
  }

  // Sleep for time specified in definition, or default 1000ms

  long timeout = 0;

  SM_DEBUG("Reading sleep config\n");

  if (_definition->containsKey(DEFINITION_SLEEP_TIMEOUT))
  {
    timeout = round(compute.evalMath((*_definition)[DEFINITION_SLEEP_TIMEOUT]));
  }

  if (timeout > 0)
  {
    SM_DEBUG("Sleep for " << timeout << "ms\n");
    _sleep((unsigned long)timeout);
  }

  SM_DEBUG("Exiting cycle\n");
}

/**************************************************************************
 *                        Private methods
 **************************************************************************/

#ifdef SM_DEBUGGER
void StateMachineController::setDebugPrinter(DebugPrinter printer)
{
  __debugPrinter = printer;
}
#endif

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
    SM_DEBUG("Try run action: " << (const char *)action << "\n");
    _runAction((const char *)action);
  }
  else if (action.is<JsonObject>())
  {
    _runActionWithParams(action.as<JsonObject>());
  }
}

void StateMachineController::_runAction(const char *actionId)
{
  // check if action is one of step machine registered actions
  if (_actionMap.count(actionId))
  {
    _actionMap[actionId](&_actionContext);
    SM_DEBUG("Action done: " << actionId << "\n");
  }
  // check if action is one of registered plugin actions
  else
  {
    _runPluginActions(actionId);
  }
}

void StateMachineController::_runActionWithParams(JsonObject actions)
{
  if (actions.isNull())
    return;

  for (JsonPair action : actions)
  {
    const char *actionId = action.key().c_str();

    JsonVariant item = action.value();
    if (!item.is<JsonArray>())
      continue;
    JsonArray params = item.as<JsonArray>();

    _actionContext.setParams(&params);
    _runAction(actionId);
    _actionContext.resetParams();
  }
}

void StateMachineController::_runPluginActions(const char *actionId)
{
  SM_DEBUG("Try run plugin action: " << actionId << "\n");

  char *pluginId, *pluginActionId;
  char buff[strlen(actionId) + 1];
  strcpy(buff, actionId);
  pluginId = strtok(buff, ".");
  pluginActionId = strtok(NULL, "");

  if (pluginId == nullptr || pluginActionId == nullptr)
    return;

  // do we have such plugin registerd ?
  if (_pluginMap.count(pluginId))
  {
    SM_DEBUG("Plugin found: " << pluginId << "\n");

    Plugin *plugin = _pluginMap[pluginId];
    // do we have this action in a plugin?
    if (plugin->actionMap.count(pluginActionId))
    {
      // run action
      plugin->actionMap[pluginActionId](plugin);
      SM_DEBUG("Plugin action done: " << pluginActionId << "\n");
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

  SM_DEBUG("Switch to state: " << newState << "\n");

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

    SM_DEBUG("Running state machine: " << _stateMachines[i].name << "\n");

    // run initial actions for each cycle

    _runActions(_stateMachines[i].machine[SM_BEFORE_CYCLE_ACTIONS]);

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
    SM_DEBUG("Checking rule: " << item << "\n");

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
    const char *targetState = rule[STATE_RULE_THEN].as<char *>();

    SM_DEBUG("Evaluate condition: " << condition << "\n");

    if (compute.evalCondition(rule[STATE_RULE_IF]))
    {
      // rule satisfied, return next state name
      SM_DEBUG("Rule satisfied, switching to state: " << targetState << "\n");
      return targetState;
    }

    SM_DEBUG("Rule not satisfied\n");
  }

  return nullptr;
}
