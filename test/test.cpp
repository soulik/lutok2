#include "lutok2/lutok2.hpp"

using namespace lutok2;

int testFun(State & state){
	state.loadString(
		"print(\"Hello world from lambda function!\") \
		"
		);
	state.stack->call(0,0);
	return 0;
}

class TestObj {
private:
	std::string value;
public:
	TestObj(const std::string & value){
		this->value = value;
	}
	std::string getValue(){
		return value;
	}
	void setValue(const std::string & value){
		this->value = value;
	}
};

class LTestObj : public Object<TestObj> {
public:
	explicit LTestObj(State * state) : Object<TestObj>(state){
		LUTOK_PROPERTY("value", &LTestObj::getValue, &LTestObj::setValue);
		LUTOK_METHOD("method", &LTestObj::method);
	}
	TestObj * constructor(State & state){
		TestObj * obj = nullptr;
		Stack * stack = state.stack;
		if (stack->is<LUA_TSTRING>(1)){
			std::string value = stack->to<std::string>(1);
			obj = new TestObj(value);
		}
		
		return obj;
	}
	void destructor(State & state, TestObj * object){
		delete object;
	}
	int operator_concat(State & state, TestObj * a, TestObj * b){
		push(new TestObj(a->getValue() + b->getValue()), true);
		return 1;
	}

	int getValue(State & state, TestObj * object){
		state.stack->push<std::string>(object->getValue());
		return 1;
	}
	int setValue(State & state, TestObj * object){
		object->setValue(state.stack->to<std::string>(2));
		return 0;
	}
	int method(State & state, TestObj * object){
		state.stack->push<std::string>("Hello");
		return 1;
	}
};


int main(char ** argv, int argc){

	State state;
	state.interfaces["testObj"] = new LTestObj(&state);
	state.openLibs();
	
	state.stack->push<Function>([](State & state) -> int{
		state.loadString(
			"print(\"Hello world from lambda function!\") \
			"
			);
		state.stack->call(0,0);
		return 0;
	});
	state.stack->setGlobal("testing");

	dynamic_cast<LTestObj*>(state.interfaces["testObj"])->getConstructor();
	state.stack->setGlobal("testObj");

	state.loadFile("test/test.lua");
	state.stack->call(0,0);
	return 0;
}