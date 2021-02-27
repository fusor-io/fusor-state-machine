#include <gtest/gtest.h>
#include <ArduinoJson.h>
#include <iostream>
#include <map>
#include <float.h>
#include <limits.h>

// #define SM_DEBUGGER

#include "../src/StateMachine.cpp"

StaticJsonDocument<1024> _doc;
char _jsonBuff[1024];

unsigned long _time = 0;
unsigned long getTime() { return _time; }

JsonVariant makeVariant(const char *json)
{
  strcpy(_jsonBuff, "{\"v\":");
  strcat(_jsonBuff, json);
  strcat(_jsonBuff, "}");
  deserializeJson(_doc, _jsonBuff);
  return _doc["v"];
}

void debugPrinter(const char *message)
{
  std::cout << message;
}



TEST(StateMachine, CreateSM)
{
  StateMachineController sm = StateMachineController("test", NULL, getTime);
}

TEST(StateMachine, NameWithScope)
{
  KeyCreate keyCreator;
  ASSERT_STREQ(keyCreator.withScope("testid", "testname"), "testid.testname");
}

TEST(StateMachine, setVar_getVarInt)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.compute.store.setVar("test", 42);
  sm.compute.store.setVar("test0", 42);
  sm.setVar("test1", 137.1f);
  ASSERT_EQ(sm.getVarInt("test"), 42);
  ASSERT_EQ(sm.getVarInt("test1"), 137);
  ASSERT_EQ(sm.getVarInt("test2"), 0);
  ASSERT_EQ(sm.getVarInt("test2", 7), 7);
}

TEST(StateMachine, updateVarInt)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  VarStruct *var = sm.compute.store.updateVar(nullptr, "test", 42);
  ASSERT_EQ(var->vInt, 42);
  ASSERT_EQ(var->vFloat, 42.0f);
  
  sm.compute.store.updateVar(var, "test", 137);
  ASSERT_EQ(var->vInt, 137);
  ASSERT_EQ(var->vFloat, 137.0f);
}

TEST(StateMachine, updateVarFloat)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  VarStruct *var = sm.compute.store.updateVar(nullptr, "test", 42.0f);
  ASSERT_EQ(var->vInt, 42);
  ASSERT_EQ(var->vFloat, 42.0f);
  
  sm.compute.store.updateVar(var, "test", 137.0f);
  ASSERT_EQ(var->vInt, 137);
  ASSERT_EQ(var->vFloat, 137.0f);
}

TEST(StateMachine, setVar_getVarFloat)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.setVar("test", 42.0f);
  ASSERT_EQ(sm.getVarFloat("test"), 42.0f);
  sm.compute.store.setVar("test2", 42);
  ASSERT_EQ(sm.getVarFloat("test2"), 42.0f);
}

long int foo1 = 0, foo2 = 0, foo3 = 0;
void dummy_action1(ActionContext *ctx) { foo1 = 42; }
void dummy_action2(ActionContext *ctx) { foo2 = 136; }
void dummy_action3(ActionContext *ctx) { foo3 = ctx->getParamInt(0); }

TEST(StateMachine, registerAction_runAction)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.registerAction("action1", &dummy_action1);

  JsonVariant action1 = makeVariant("\"action1\"");

  foo1 = 0;
  sm._runAction(action1);
  ASSERT_EQ(foo1, 42);

  // test non existent action
  JsonVariant action2 = makeVariant("\"action2\"");

  foo1 = 0;
  sm._runAction(action2);
  ASSERT_EQ(foo1, 0);
}

TEST(StateMachine, registerAction_runActionWitParams)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.registerAction("action3", &dummy_action3);

  JsonVariant action3 = makeVariant("{\"action3\":[{ \"sum\": [\"var1\",1]}]}");
  sm.compute.store.setVar("var1", 136);

  foo3 = 0;
  sm._runAction(action3);
  ASSERT_EQ(foo3, 137);

  // test non existent action
  JsonVariant action2 = makeVariant("{\"action2\":[137]}");

  foo2 = 0;
  sm._runAction(action2);
  ASSERT_EQ(foo2, 0);
}

TEST(StateMachine, registerAction_runAssignmentAction)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);

  sm.compute.store.setVar("var1", 136);
  long int var1 = sm.compute.store.getVarInt("var1");
  ASSERT_EQ(var1, 136);

  // should evaluate and assign
  JsonVariant action1 = makeVariant("{\":=\":[\"var1\", { \"sum\": [1,41]}]}");
  sm._runAction(action1);

  var1 = sm.compute.store.getVarInt("var1");
  ASSERT_EQ(var1, 42);

  // should not fail with bad data
  JsonVariant action2 = makeVariant("{\":=\":[\"var1\"]}");
  sm._runAction(action2);

  var1 = sm.compute.store.getVarInt("var1");
  ASSERT_EQ(var1, 42);

  JsonVariant action3 = makeVariant("{\":=\":\"\"}");
  sm._runAction(action3);

  var1 = sm.compute.store.getVarInt("var1");
  ASSERT_EQ(var1, 42);
}

TEST(StateMachine, runActions)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.registerAction("action1", &dummy_action1);
  sm.registerAction("action2", &dummy_action2);

  JsonVariant actions = makeVariant("[\"action1\",\"action2\"]");

  foo2 = foo1 = 0;
  sm._runActions(actions);
  ASSERT_EQ(foo1, 42);
  ASSERT_EQ(foo2, 136);
}

TEST(StateMachine, evalMath)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  JsonVariant expression;

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sqrt\":[16]}")), 4.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sqrt\":[-1]}")), FLT_MIN);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"SQRT\":[16]}")), 4.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"Sqrt\":[16]}")), 4.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sqrt\":\"\"}")), 0.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sqrt\":[]}")), 0.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sqrt\":[16,17]}")), 4.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"exp\":[1]}")), 2.71828182846f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"exp\":[0]}")), 1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"exp\":[-1]}")), 0.36787944117f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"ln\":[1]}")), 0.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"ln\":[10]}")), 2.30258509299f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"ln\":[-1]}")), FLT_MIN);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"log\":[1]}")), 0.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"log\":[10]}")), 1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"log\":[-1]}")), FLT_MIN);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"abs\":[1]}")), 1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"abs\":[-1]}")), 1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"abs\":[0]}")), 0.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"neg\":[1]}")), -1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"neg\":[-1]}")), 1.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"neg\":[0]}")), 0.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sub\":[43,1]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"div\":[84,2]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"pow\":[2,10]}")), 1024.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sum\":[42]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sum\":[3.0, 0.14]}")), 3.14f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sum\":[4.0, -1.0, 0.1, 0.04]}")), 3.14f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"mul\":[42]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"mul\":[2,21]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"mul\":[2,3,7]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"min\":[42]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"min\":[42,43]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"min\":[43,42,44]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"max\":[42]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"max\":[41,42]}")), 42.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"max\":[40,42,41]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"sum\":[{\"sqrt\":[64]},{\"mul\":[2,17]}]}")), 42.0f);

  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"diff\":[10,20]}")), 10.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"diff\":[20,10]}")), 10.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"diff\":[-10,10]}")), 20.0f);
  ASSERT_FLOAT_EQ(sm.compute.evalMath(makeVariant("{\"diff\":[10,-10]}")), 20.0f);
}

TEST(StateMachine, switchCondition)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  JsonVariant operands;

  ASSERT_TRUE(sm.compute.switchCondition("not", makeVariant("[false]")));
  ASSERT_TRUE(sm.compute.switchCondition("not", makeVariant("[false,true]")));
  ASSERT_TRUE(sm.compute.switchCondition("not", makeVariant("false")));
  ASSERT_FALSE(sm.compute.switchCondition("not", makeVariant("[true]")));
  ASSERT_FALSE(sm.compute.switchCondition("not", makeVariant("true")));
  ASSERT_TRUE(sm.compute.switchCondition("not", makeVariant("\"\"")));
  ASSERT_TRUE(sm.compute.switchCondition("not", makeVariant("")));

  ASSERT_TRUE(sm.compute.switchCondition("and", makeVariant("[true]")));
  ASSERT_TRUE(sm.compute.switchCondition("and", makeVariant("[true,true]")));
  ASSERT_TRUE(sm.compute.switchCondition("and", makeVariant("[true,true,true]")));
  ASSERT_FALSE(sm.compute.switchCondition("and", makeVariant("[true,false,true]")));

  ASSERT_TRUE(sm.compute.switchCondition("or", makeVariant("[true]")));
  ASSERT_TRUE(sm.compute.switchCondition("or", makeVariant("[true,true]")));
  ASSERT_TRUE(sm.compute.switchCondition("or", makeVariant("[true,false,true]")));
  ASSERT_FALSE(sm.compute.switchCondition("or", makeVariant("[false,false,false]")));

  ASSERT_FALSE(sm.compute.switchCondition("gt", makeVariant("[]")));
  ASSERT_FALSE(sm.compute.switchCondition("gt", makeVariant("[42]")));
  ASSERT_TRUE(sm.compute.switchCondition("gt", makeVariant("[137,42]")));
  ASSERT_FALSE(sm.compute.switchCondition("gt", makeVariant("[42,137]")));
  ASSERT_TRUE(sm.compute.switchCondition("gt", makeVariant("[137,42,777]")));

  ASSERT_TRUE(sm.compute.switchCondition("gte", makeVariant("[42,42]")));
  ASSERT_TRUE(sm.compute.switchCondition("gte", makeVariant("[137,42]")));
  ASSERT_FALSE(sm.compute.switchCondition("gte", makeVariant("[42,137]")));

  ASSERT_TRUE(sm.compute.switchCondition("lt", makeVariant("[42,137]")));
  ASSERT_FALSE(sm.compute.switchCondition("lt", makeVariant("[137,42]")));

  ASSERT_TRUE(sm.compute.switchCondition("lte", makeVariant("[42,42]")));
  ASSERT_TRUE(sm.compute.switchCondition("lte", makeVariant("[42,137]")));
  ASSERT_FALSE(sm.compute.switchCondition("lte", makeVariant("[137,42]")));

  ASSERT_TRUE(sm.compute.switchCondition("eq", makeVariant("[42,42]")));
  ASSERT_FALSE(sm.compute.switchCondition("eq", makeVariant("[42,137]")));

  ASSERT_TRUE(sm.compute.switchCondition("ne", makeVariant("[42,137]")));
  ASSERT_FALSE(sm.compute.switchCondition("ne", makeVariant("[42,42]")));
}

TEST(StateMachine, evalCondition)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  JsonVariant query;

  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("true")));
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("false")));

  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("42")));
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("0")));

  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("42.0")));
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("0.0")));

  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("")));
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("\"\"")));
  sm.compute.store.setVar("test1", 42);
  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("\"test1\"")));
  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("\"sm.test1\"")));
  sm.compute.store.setVar("test2", 0);
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("\"sm.test2\"")));
  ASSERT_FALSE(sm.compute.evalCondition(makeVariant("\"sm.test3\"")));

  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("{\"gt\":[137,42]}")));

  ASSERT_TRUE(sm.compute.evalCondition(makeVariant("{\
    \"and\":[\
       {\"lt\":\
          [\
             {\"sub\":[42,1]},\
             \"test1\"\
	        ]\
       },\
       {\"eq\": [\"test2\",0]}\
   ]}")));
}

TEST(StateMachine, evalCondition_elapsed)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  JsonVariant query;

  JsonVariant conditions = makeVariant(
      "[{\"elapsed\":[\"timer1\", 100]},{\"elapsed\":[\"timer2\", 1000]},{\"elapsed\":[\"timer3\", 2]}");

  JsonVariant cond100 = conditions[0];
  JsonVariant cond1000 = conditions[1];
  JsonVariant cond2 = conditions[2];

  ASSERT_FALSE(sm.compute.evalCondition(cond100));
  ASSERT_FALSE(sm.compute.evalCondition(cond1000));

  _time = 99;
  ASSERT_FALSE(sm.compute.evalCondition(cond100));
  ASSERT_FALSE(sm.compute.evalCondition(cond1000));

  _time = 100;
  ASSERT_TRUE(sm.compute.evalCondition(cond100));
  ASSERT_FALSE(sm.compute.evalCondition(cond100));
  ASSERT_FALSE(sm.compute.evalCondition(cond1000));

  _time = 1000;
  ASSERT_TRUE(sm.compute.evalCondition(cond100));
  ASSERT_TRUE(sm.compute.evalCondition(cond1000));

  _time = ULONG_MAX;
  ASSERT_FALSE(sm.compute.evalCondition(cond2));
  _time = 0;
  ASSERT_FALSE(sm.compute.evalCondition(cond2));
  _time = 1;
  ASSERT_TRUE(sm.compute.evalCondition(cond2));
}

void sm_init_action(ActionContext *ctx) { ctx->compute->store.setVar("init", 1); }
void sm_before_action(ActionContext *ctx) { ctx->compute->store.setVar("before", 1); }
void sm_after_action(ActionContext *ctx) { ctx->compute->store.setVar("after", 1); }
void sm1_action1(ActionContext *ctx) { ctx->compute->store.setVar("var1", 1); }
void sm1_action2(ActionContext *ctx)
{
  int v = ctx->compute->getVarInt("var1");
  ctx->compute->store.setVar("var1", v + 42);
}
void sm1_action3(ActionContext *ctx) { ctx->compute->store.setVar("var1", 77); }
void sm1_action4(ActionContext *ctx)
{
  ctx->compute->store.setVar("each_cycle", 137);
}

TEST(StateMachine, lifeCycle)
{
  const char *testSMJson = "{\
   \"i\":[\"init_action\"],\
   \"b\":[\"before_action\"],\
   \"a\":[\"after_action\"],\
   \"t\":1000,\
   \"s\":{\
    \"sm1\": {\
      \"i\": \"sm1_state1\",\
      \"a\": [\"sm1_action1\"],\
      \"b\": [\"sm1_action4\"],\
      \"s\": {\
        \"sm1_state1\": {\
	        \"a\": [\"sm1_action2\"],\
	        \"r\": [\
            {\
              \"i\":\
                {\"gt\": [\"sm.var1\", 42]},\
              \"t\": \"sm1_state2\"\
            }\
    	   ]\
        },\
        \"sm1_state2\": {\
          \"a\": [\"sm1_action3\"],\
          \"r\": [\
            {\
              \"i\":\
                {\"lt\": [\"sm.var1\", 136]},\
              \"t\": \"sm1_state1\"\
            }\
          ]\
        }\
      }\
    }\
   }\
  }";

  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, testSMJson);
  sm.setDefinition(&doc);
#ifdef SM_DEBUGGER
  sm.setDebugPrinter(debugPrinter);
#endif
  sm.registerAction("init_action", sm_init_action);
  sm.registerAction("before_action", sm_before_action);
  sm.registerAction("after_action", sm_after_action);
  sm.registerAction("sm1_action1", sm1_action1);
  sm.registerAction("sm1_action2", sm1_action2);
  sm.registerAction("sm1_action3", sm1_action3);
  sm.registerAction("sm1_action4", sm1_action4);
  sm.init();

  // check if controller init action was called
  ASSERT_EQ(sm.getVarInt("init"), 1);

  // check if machine(s) are stored
  ASSERT_EQ(sm._stateMachineCount, 1);

  // check if machine state is initialized
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state1");

  // check if
  //   initial state machine action is called
  //   initial state action is called
  ASSERT_EQ(sm.getVarInt("var1"), 43);

  // check if before and after actions are called when running cycle
  ASSERT_EQ(sm.getVarInt("before"), 0);
  ASSERT_EQ(sm.getVarInt("after"), 0);
  sm.cycle();
  ASSERT_EQ(sm.getVarInt("before"), 1);
  ASSERT_EQ(sm.getVarInt("after"), 1);
  ASSERT_EQ(sm.getVarInt("each_cycle"), 137);

  // check if machine is changed its state
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state2");
  ASSERT_EQ(sm.getVarInt("var1"), 77);

  sm.cycle();
  // check if machine is changed its state again
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state1");
  ASSERT_EQ(sm.getVarInt("var1"), 77 + 42);
}

void pluginAction(Plugin *pl)
{
  int var = pl->getVarInt("pl_var1");
  pl->setVar("pl_var1", var + 42);
}

void pluginAction2(Plugin *pl)
{
  int var = pl->getVarInt("pl_var2");
  pl->setVar("pl_var2", var + 137);
}

TEST(StateMachine, plugin)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  Plugin testPlugin("plugin");
  testPlugin.registerAction("act1", pluginAction);
  testPlugin.registerAction("act2", pluginAction2);
  sm.registerPlugin(&testPlugin);

  JsonVariant actions = makeVariant("[\"plugin.act1\",\"plugin.act2\"]");
  sm._runActions(actions);

  ASSERT_EQ(sm.getVarInt("sm.plugin.pl_var1"), 42);
  ASSERT_FLOAT_EQ(sm.getVarFloat("sm.plugin.pl_var2"), 137.0f);
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
