#include "common.hpp"

#define LUTOK2_NOT_USED(x) ((void)(x))

namespace lutok2 {
	class State;

	typedef int (*cxx_function)(State &);
	typedef std::function<int(State &)> Function;

	typedef std::unordered_map<std::string, cxx_function> module;
	static int cxx_function_wrapper(lua_State *);

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

	static State * getCurrentState(lua_State * L){
		State * state = nullptr;
		luaL_getmetatable(L, "_lutok2");
		if (lua_type(L, -1) == LUA_TTABLE){
			lua_getfield(L, -1, "State");
			if (lua_type(L, -1) == LUA_TUSERDATA){
				state = *(static_cast<State **>(lua_touserdata(L, -1)));
				lua_pop(L, 2);
			}else{
				lua_pop(L, 2);
			}
		}else{
			lua_pop(L, 1);
		}
		assert(state != nullptr);
		return state;
	}

	static int cxx_function_wrapper(lua_State * L){
		State * state = getCurrentState(L);
		Function * originalFunction = *(static_cast<Function ** >(state->stack->to<void*>(state->stack->upvalueIndex(1))));
		try{
			return (*originalFunction)(*state);
		}catch(const std::exception & e){
			state->error("Unhandled exception: %s", e.what());
			return 1;
		}
	}

	static int free_current_state(lua_State * L){
		State * state = getCurrentState(L);
		//free all stored object interfaces
		for (std::unordered_map<std::string, BaseObject*>::iterator iter = state->interfaces.begin(); iter != state->interfaces.end(); iter++){
			delete (iter->second);
		}

		delete state;
		return 0;
	}
};
