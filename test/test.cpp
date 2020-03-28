#include <gtest/gtest.h>
#include <ArduinoJson.h>
#include <iostream>
#include <map>
#include <float.h>
#include <limits.h>

#include "../src/smcontroller.cpp"

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

TEST(StateMachine, CreateSM)
{
  StateMachineController sm = StateMachineController("test", NULL, getTime);
}

TEST(StateMachine, NameWithScope)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  ASSERT_STREQ(sm.compute.store.nameWithScope("test"), "sm.test");
}

TEST(StateMachine, setVar_getVarInt)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.compute.store.setVar("test", 42);
  ASSERT_EQ(sm.compute.store.getVarInt("test"), 42);
  sm.compute.store.setVar("test1", 42.1f);
  ASSERT_EQ(sm.compute.store.getVarInt("test1"), 42);
  ASSERT_EQ(sm.compute.store.getVarInt("test2"), 0);
  ASSERT_EQ(sm.compute.store.getVarInt("test2", 7), 7);
}

TEST(StateMachine, setVar_getVarFloat)
{
  StateMachineController sm = StateMachineController("sm", NULL, getTime);
  sm.compute.store.setVar("test", 42.0f);
  ASSERT_EQ(sm.compute.store.getVarFloat("test"), 42.0f);
  sm.compute.store.setVar("test2", 42);
  ASSERT_EQ(sm.compute.store.getVarFloat("test2"), 42.0f);
}

int foo1 = 0, foo2 = 0;
void dummy_action1(StateMachineController *smc) { foo1 = 42; }
void dummy_action2(StateMachineController *smc) { foo2 = 136; }

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

void sm_init_action(StateMachineController *smc) { smc->compute.store.setVar("init", 1); }
void sm_before_action(StateMachineController *smc) { smc->compute.store.setVar("before", 1); }
void sm_after_action(StateMachineController *smc) { smc->compute.store.setVar("after", 1); }
void sm1_action1(StateMachineController *smc) { smc->compute.store.setVar("var1", 1); }
void sm1_action2(StateMachineController *smc)
{
  int v = smc->compute.store.getVarInt("var1");
  smc->compute.store.setVar("var1", v + 42);
}
void sm1_action3(StateMachineController *smc) { smc->compute.store.setVar("var1", 77); }

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
  sm.registerAction("init_action", sm_init_action);
  sm.registerAction("before_action", sm_before_action);
  sm.registerAction("after_action", sm_after_action);
  sm.registerAction("sm1_action1", sm1_action1);
  sm.registerAction("sm1_action2", sm1_action2);
  sm.registerAction("sm1_action3", sm1_action3);
  sm.init();

  // check if controller init action was called
  ASSERT_EQ(sm.compute.store.getVarInt("init"), 1);

  // check if machine(s) are stored
  ASSERT_EQ(sm._stateMachineCount, 1);

  // check if machine state is initialized
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state1");

  // check if
  //   initial state machine action is called
  //   initial state action is called
  ASSERT_EQ(sm.compute.store.getVarInt("var1"), 43);

  // check if before and after actions are called when running cycle
  ASSERT_EQ(sm.compute.store.getVarInt("before"), 0);
  ASSERT_EQ(sm.compute.store.getVarInt("after"), 0);
  sm.cycle();
  ASSERT_EQ(sm.compute.store.getVarInt("before"), 1);
  ASSERT_EQ(sm.compute.store.getVarInt("after"), 1);

  // check if machine is changed its state
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state2");
  ASSERT_EQ(sm.compute.store.getVarInt("var1"), 77);

  sm.cycle();
  // check if machine is changed its state again
  ASSERT_STREQ(sm._stateMachines[0].state, "sm1_state1");
  ASSERT_EQ(sm.compute.store.getVarInt("var1"), 77 + 42);
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
