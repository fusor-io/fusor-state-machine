/*
  State Machine Controller -
  library for running configuration based state machines on Arduino
  Copyright Giedrius Lukosevicius 2020
  MIT License
*/

#ifndef statemachine_h
#define statemachine_h

// Uncomment the following line and recompile library
// to enable debugg printing
// #define SM_DEBUGGER

#define MAX_VAR_NAME_LEN 32     // maximum length of variable name ("device-id.var-name.type")
#define MAX_VARIABLE_SPACE 1024 // maximum size of JSON storing local variables
#define MAX_STATE_MACHINES 16

#define DEFINITION_INIT_ACTION "i"
#define DEFINITION_BEFORE_ACTION "b"
#define DEFINITION_AFTER_ACTION "a"
#define DEFINITION_STATE_MACHINES "s"
#define DEFINITION_SLEEP_TIMEOUT "t"

#define STATE_ENTRY_ACTIONS "a"
#define STATE_EXIT_RULES "r"
#define STATE_RULE_IF "i"
#define STATE_RULE_THEN "t"

#define SM_INITIAL_STATE "i"
#define SM_INITIAL_ACTIONS "a"
#define SM_BEFORE_CYCLE_ACTIONS "b"
#define SM_STATES "s"

class StateMachineController; // forward declaration

#include <map>

#include <ArduinoJson.h>
// See: https://arduinojson.org/v6/api/

#include "keycompare/keycompare.h"
#include "timers/timers.h"
#include "store/store.h"
#include "store/varStruct.h"
#include "compute/compute.h"
#include "plugin/plugin.h"
#include "actioncontext/actioncontext.h"
#include "hooks/hooks.h"

#include "StateMachineDebug.h"

/*
 * Controller definitions should be an DynamicJsonDocument of the following structure:
 * {
 *  DEFINITION_INIT_ACTION:    ["action_name_1"],         // actions to execute when initializing device
 *  DEFINITION_BEFORE_ACTION:  ["action_name_2"],         // actions to execute starting the cycle
 *  DEFINITION_STATE_MACHINES: { "machine1": machine},    // definitions of actual state machines
 *  DEFINITION_AFTER_ACTION:   ["action_name_3"],         // actions to execute after the cycle
 *  DEFINITION_SLEEP_TIMEOUT:  "ctrl.var_name_1"          // variable defining sleep between cycles, default 1000 (ms)
 * }
 * 
 * State Machines definition example with one machine "fan": 
 * {
 *    "temperature": {
 *        [SM_INITIAL_STATE]: "start",
 *        [SM_INITIAL_ACTIONS]: ["setup_sensor"],
 *        [SM_STATES]: {
 *          "start": {
 *             [STATE_ENTRY_ACTIONS]: ["turn_sensor_on",],
 *             [STATE_EXIT_RULES]: [
 *               { [STATE_RULE_IF]: {"elapsed": ["timer1", 1000]}, [STATE_RULE_THEN]: "read" }
 *             ]
 *          },
 *          "read": {
 *             [STATE_ENTRY_ACTIONS]: ["read_sensor", "turn_sensor_off"],
 *             [STATE_EXIT_RULES]: [
 *               { [STATE_RULE_IF]: {"elapsed", ["timer1", "cfg.timeout"]}, [STATE_RULE_THEN]: "start" }
 *             ]
 *           }
 *        }
 *    },
 *    "fan": {
 *      [SM_INITIAL_STATE]: "idle",
 *      [SM_INITIAL_ACTIONS]: ["disable_fan"],
 *      [SM_STATES]: {
 *        "idle": { 
 *           [STATE_ENTRY_ACTIONS]: ["disable_fan"],
 *           [STATE_EXIT_RULES]: [
 *             { [STATE_RULE_IF]: condition_1, [STATE_RULE_THEN]: "fan_on" }
 *           ]
 *        }, 
 *        "fan_on": {
 *           [STATE_ENTRY_ACTIONS]: ["enable_fan"],
 *           [STATE_EXIT_RULES]: [
 *             { [STATE_RULE_IF]: condition_2, [STATE_RULE_THEN]: "idle" }
 *           ]
 *        }
 *      }
 *    }
 * }
 *  
 * Condition structure example: 
 * {
 *   "and": [
 *     {
 *       "gt": ["garage.humidity", 30]
  *     },
 *     {
 *       "gt": ["fan-off-time", "ctrl.fan-min-off"]
 *     }
 *   ]
 * }
 *  
 * Variables are of the following structure:
 * { 
 *   "[scope.][plugin.]var-name": <value>
 * }
 * 
 * Example:
 * {
 *   "ctrl.max-duration": 3600,    // external
 *   "weather.bme280.temp": 25.5,  // from plugin
 *   "humidity": 40                // internal
 * }
 * 
 * Actions can be 
 *   - simple, represented by name (text string), ex. "my-action"
 *   - with params, represented by JsonObject, ex.
 *     {
 *       "my-action": [123, "my-var-1", {"sqrt":"my-var-2"}]
 *     } 
 */

typedef struct state_machine_slot
{
  const char *name;
  const char *state;
  JsonObject machine;
  JsonObject states_definition;
} STATE_MACHINE_SLOT;

// callback declarations
typedef void (*ActionFunction)(ActionContext *);
typedef void (*SleepFunction)(unsigned long);
typedef unsigned long (*GetTimeFunction)(void);

class StateMachineController
{
public:
  Timers timers;
  Compute compute;
  unsigned long cycleNum = 0;

  StateMachineController(const char *, SleepFunction, GetTimeFunction);
  void setActionRunner(ActionFunction);
  void registerAction(const char *, ActionFunction);
  void registerPlugin(Plugin *);
  void setDefinition(JsonDocument *);
  void setDefinition(JsonVariant);
  void init();
  void cycle();
  void setHooks(Hooks *);

#ifdef SM_DEBUGGER
  void setDebugPrinter(DebugPrinter);
#endif

  // private:
  const char *_deviceId;
  Hooks *_hooks = nullptr;

  JsonVariant _definition; // definition of the controller

  int _stateMachineCount = 0;
  STATE_MACHINE_SLOT _stateMachines[MAX_STATE_MACHINES];

  std::map<const char *, ActionFunction, KeyCompare> _actionMap;
  std::map<const char *, Plugin *, KeyCompare> _pluginMap;

  SleepFunction _sleepCallback;
  void _yield();
  void _sleep(unsigned long);

  void _runAction(JsonVariant);
  void _runAction(const char *);
  void _runActions(JsonVariant);
  void _runActionWithParams(JsonObject);
  void _runPluginActions(const char *);
  void _runInitAction();
  void _initStateMachines();
  void _runStateMachines();
  void _switchState(STATE_MACHINE_SLOT *, const char *);
  const char *_getNextState(JsonArray);

  ActionContext _actionContext;
};

#endif
