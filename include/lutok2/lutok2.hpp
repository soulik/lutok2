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
		State * state = State::getCurrentState();
		int upvalueIndex = state->stack->upvalueIndex(1);
		Function ** originalFunction = reinterpret_cast<Function ** >(state->stack->to<void*>(upvalueIndex));
		if (originalFunction != nullptr){
			//Function * originalFunction = *pOriginalFunction;
			//Function * originalFunction = *(static_cast<Function ** >(state->stack->to<void*>(upvalueIndex)));
			try{
				return (**originalFunction)(*state);
			}catch(const std::exception & e){
				state->error("Unhandled exception: %s", e.what());
				return 1;
			}
		}else{
			const std::string traceBack = state->traceback();
			state->error("Invalid function pointer!: %s", traceBack.c_str());
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
