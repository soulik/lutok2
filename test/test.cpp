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
	TestObj * constructor(State & state, bool & managed){
		TestObj * obj = nullptr;
		Stack * stack = state.stack;
		if (stack->is<LUA_TSTRING>(1)){
			const std::string value = stack->to<const std::string>(1);
			obj = new TestObj(value);
		}
		managed = true;
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
		state.stack->push<const std::string &>(object->getValue());
		return 1;
	}
	int setValue(State & state, TestObj * object){
		const std::string value = state.stack->to<const std::string>(1);
		object->setValue(value);
		return 0;
	}
	int method(State & state, TestObj * object){
		state.stack->push<const std::string &>("Hello");
		return 1;
	}
};


int main(char ** argv, int argc){

	State state;
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

	state.registerInterface<LTestObj>("testObj");
	state.stack->setGlobal("testObj");

	try {
		state.loadFile("test/test.lua");
		state.stack->call(0,0);
	}catch(std::exception & e){
		printf("Can't load test file: %s", e.what());
	}
	return 0;
}