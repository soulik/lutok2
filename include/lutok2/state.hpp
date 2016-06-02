#ifndef LUTOK2_STATE_H
#define LUTOK2_STATE_H

namespace lutok2 {
	class StackDebugger;
	class BaseObject;

	struct StateData {
		std::unordered_map<std::string, BaseObject*> interfaces;
	};

	class State {
	friend class Stack;
	public:
		Stack * stack;
		lua_State * state;
		lua_State * originalState;
	private:
		bool owned;

		inline const char * findTable(const int index, const std::string & name, int szHint){
			return luaL_findtable(state, index, name.c_str(), szHint);
		}
		
		void findLib(const std::string & name, const size_t size, const int nup){
			findTable(LUA_REGISTRYINDEX, "_LOADED", 1);
			stack->getField(name, -1);  /* get _LOADED[name] */
			if (!stack->is<LUA_TTABLE>()) {  /* not found? */
				stack->pop();  /* remove previous result */
				/* try global variable (and create one if it does not exist) */
				if (findTable(LUA_GLOBALSINDEX, name, static_cast<int>(size) ) != NULL)
					error("Name conflict for module " LUA_QS, name.c_str());
				stack->pushValue(-1);
				stack->setField(name, -3);  /* _LOADED[name] = new table */
			}
			stack->remove(-2);  /* remove _LOADED table */
			stack->insert(-(nup+1));  /* move library table to below upvalues */
		}
	public:
		State(bool lua_managed = false){
			originalState = nullptr;
			newState();
			initState(lua_managed);
		}

		explicit State(lua_State * state, bool lua_managed = true){
			this->state = state;
			originalState = nullptr;
			owned = false;
			initState(lua_managed);
		}

		~State(){
			if (owned){
				closeState();
			}
			delete stack;
		}

		State & operator= (State & arg){
			state = arg.state;
			owned = arg.owned;
			return *this;
		}

		bool operator== (State & arg){
			return (arg.state == state);
		}
		/*
		inline static State * getCurrentState(State * newStateInstance = nullptr){
			static __thread_local State * stateInstance = nullptr;
			if (newStateInstance){
				stateInstance = newStateInstance;
			}
			return stateInstance;
		}
		*/

		void initState(bool lua_managed = false){
			stack = new Stack(&state, &originalState, this);
			/*
			if (lua_managed){
				getCurrentState(this);
			}
			*/
		}

		void newState(){
			state = luaL_newstate();
			owned = true;
		}

		void closeState(){
			if (state != nullptr){
				lua_close(state);
				state = nullptr;
			}
		}

		void openLibs(){
			luaL_openlibs(state);
		}

		/*
			Loaders
		*/

		void loadFile(const std::string & fileName){
			int rc = luaL_loadfile(state, fileName.c_str());
			if (rc != 0){
				std::string errorMessage;

				if (rc == LUA_ERRSYNTAX){
					errorMessage = "Syntax error";
				}
				else if (rc == LUA_ERRMEM){
					errorMessage = "Memory allocation error";
				}
				else if (rc == LUA_ERRFILE){
					errorMessage = "Can't open file: " + fileName;
				}
				else{
					errorMessage = "Unknown error";
				}
				if (lua_type(state, -1) == LUA_TSTRING){
					const char * luaMessage = lua_tostring(state, -1);
					errorMessage = errorMessage + "\n" + luaMessage;
				}
				throw std::runtime_error(errorMessage);
			}
		}

		void loadString(const std::string & chunk){
			int rc = luaL_loadstring(state, chunk.c_str());
			if (rc != 0){
				std::string errorMessage;

				if (rc == LUA_ERRSYNTAX){
					errorMessage = "Syntax error";
				}
				else if (rc == LUA_ERRMEM){
					errorMessage = "Memory allocation error";
				}
				else{
					errorMessage = "Unknown error";
				}
				if (lua_type(state, -1) == LUA_TSTRING){
					const char * luaMessage = lua_tostring(state, -1);
					errorMessage = errorMessage + "\n" + luaMessage;
				}
				throw std::runtime_error(errorMessage);
			}
		}

		/*
			Libraries
		*/

		void registerLib(const Module & members){
			assert(stack->is<LUA_TTABLE>());

			for (std::unordered_map< std::string, cxx_function >::const_iterator
				iter = members.begin(); iter != members.end(); iter++) {
					stack->push<const std::string &>((*iter).first);
					stack->push<cxx_function>((*iter).second);
					stack->setTable(-3);
			}
		}

		void registerLib(const Module & members, const std::string & name, const int nup=0){
			findLib(name, members.size(), nup);
			registerLib(members);
			stack->pop(nup);
		}

		/*
			Errors
		*/
		void error(const char * fmt, ...){
			char buffer[1024];
			va_list args;
			va_start (args, fmt);
#if defined(_WIN32) && defined(_MSC_VER)
			vsprintf_s(buffer, fmt, args);
#else
			vsprintf(buffer, fmt, args);
#endif
			luaL_error(state, "%s", buffer);
			va_end (args);
		}
		
		/*
			Class interfaces
		*/

		static StateData * getLocalStateData(){
			static __thread_local StateData * localStateData = nullptr;
			if (!localStateData) {
				localStateData = new StateData;
			}
			return localStateData;
		}

		template<class C> void registerInterface(const std::string & name){
			BaseObject * _interface = new C(this);
			StateData * localStateData = getLocalStateData();
			localStateData->interfaces[name] = _interface;
			_interface->getConstructor();
		}

		void registerInterface(const std::string & name, BaseObject * _interface){
			StateData * localStateData = getLocalStateData();
			localStateData->interfaces[name] = _interface;
			_interface->getConstructor();
		}
		
		template<class C> C * getInterface(const std::string & name){
			StateData * localStateData = getLocalStateData();
			C * iface = dynamic_cast<C*>(localStateData->interfaces[name]);
			assert(iface);
			return iface;
		}

		/*
			Misc
		*/

		const lua_Debug getInfo(const std::string & what){
			lua_Debug debugInfo;
			int rc = lua_getstack(state, 0, &debugInfo);
			assert(rc == 1);
			rc = lua_getinfo(state, what.c_str(), &debugInfo);
			assert(rc != 0);
			return debugInfo;
		}

		const lua_Debug getStack(const int level){
			lua_Debug debugInfo;
			int rc = lua_getstack(state, level, &debugInfo);
			assert(rc == 1);
			return debugInfo;
		}

		const std::string traceback() {
			lua_Debug info;
			int level = 0;
			std::string outputTraceback;
			char buffer[4096];

			while (lua_getstack(state, level, &info)) {
				lua_getinfo(state, "nSl", &info);

#if defined(_WIN32) && defined(_MSC_VER)
				sprintf_s(buffer, "  [%d] %s:%d -- %s [%s]\n",
					level, info.short_src, info.currentline,
					(info.name ? info.name : "<unknown>"), info.what);
#else
				sprintf(buffer, "  [%d] %s:%d -- %s [%s]\n",
					level, info.short_src, info.currentline,
					(info.name ? info.name : "<unknown>"), info.what);
#endif
				outputTraceback.append(buffer);
				++level;
			}
			return outputTraceback;
		}
	};
};

#endif