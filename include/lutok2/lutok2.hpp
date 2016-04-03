#include "common.hpp"

#define LUTOK2_NOT_USED(x) ((void)(x))

#ifdef _MSC_VER
# define __thread_local __declspec(thread) 
#else
# define __thread_local __thread
#endif

namespace lutok2 {
	class State;

	typedef int (*cxx_function)(State &);
	typedef std::function<int(State &)> Function;

	typedef std::unordered_map<std::string, cxx_function> Module;
	static int cxx_function_wrapper(lua_State *);
	static void storeCurrentState(State *, bool);
	static int free_current_state(lua_State *);

	class BaseObject {
	public:
		virtual void getConstructor(){

		}
	};

};

#include "exceptions.hpp"
#include "stack.hpp"
#include "state.hpp"
#include "stackdebugger.hpp"
#include "object.hpp"

namespace lutok2 {

	static int cxx_function_wrapper(lua_State * L) {
		State * state = reinterpret_cast<State*>(lua_touserdata(L, lua_upvalueindex(1)));
		Stack * stack = state->stack;
		/*
		State * state1 = State::getCurrentState();
		Stack * stack1 = state1->stack;
		const lua_Debug info1 = state1->getInfo("u");

		State _state2 = State(L, false);
		State * state2 = &_state2;
		Stack * stack2 = state2->stack;
		const lua_Debug info2 = state2->getInfo("u");

		State * state = state2;
		Stack * stack = stack2;
		*/
		const lua_Debug info = state->getInfo("u");

		int upvalues = info.nups;
		if (upvalues >= 1){
			int upvalueIndex = stack->upvalueIndex(2);
			stack->pushValue(upvalueIndex);
			int t = stack->type(-1);
			if (t == LUA_TUSERDATA){
				Function ** originalFunction = reinterpret_cast<Function **>(stack->to<void*>(-1));
				stack->pop(1);
				if (originalFunction != nullptr){
					try{
						return (**originalFunction)(*state);
					}
					catch (const std::exception & e){
						state->error("Unhandled exception: %s", e.what());
						return 1;
					}
				}
				else{
					const std::string traceBack = state->traceback();
					state->error("Invalid function pointer!: %s", traceBack.c_str());
					return 1;
				}
			}
			else{
				const std::string traceBack = state->traceback();
				state->error("Stack corrupted!: %s", traceBack.c_str());
				return 1;
			}
		}
		else{
			const std::string traceBack = state->traceback();
			state->error("Closure upvalues corrupted!: %s", traceBack.c_str());
			return 1;
		}
	}

	static int free_current_state(lua_State * L){
		State * state = State::getCurrentState();
		//free all stored object interfaces
		for (std::unordered_map<std::string, BaseObject*>::iterator iter = state->interfaces.begin(); iter != state->interfaces.end(); iter++){
			delete (iter->second);
		}

		delete state;
		return 0;
	}
};
