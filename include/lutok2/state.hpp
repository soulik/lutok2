#ifndef LUTOK2_STATE_H
#define LUTOK2_STATE_H

namespace lutok2 {
	class StackDebugger;
	class BaseObject;

	class State {
	friend class Stack;
	public:
		Stack * stack;
		std::unordered_map<std::string, BaseObject*> interfaces;
	private:
		lua_State * state;
		bool owned;

		inline const char * findTable(const int index, const std::string & name, int szHint){
			return luaL_findtable(state, index, name.c_str(), szHint);
		}
		
		void findLib(const std::string & name, const int size, const int nup){
			findTable(LUA_REGISTRYINDEX, "_LOADED", 1);
			stack->getField(name, -1);  /* get _LOADED[name] */
			if (!stack->is<LUA_TTABLE>()) {  /* not found? */
				stack->pop();  /* remove previous result */
				/* try global variable (and create one if it does not exist) */
				if (findTable(LUA_GLOBALSINDEX, name, size ) != NULL)
					error("Name conflict for module " LUA_QS, name);
				stack->pushValue(-1);
				stack->setField(name, -3);  /* _LOADED[name] = new table */
			}
			stack->remove(-2);  /* remove _LOADED table */
			stack->insert(-(nup+1));  /* move library table to below upvalues */
		}
	public:
		State(bool lua_managed = false){
			newState();
			initState(lua_managed);
		}

		explicit State(lua_State * state, bool lua_managed = true){
			this->state = state;
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
		}

		bool operator== (State & arg){
			return (arg.state == state);
		}

		void initState(bool lua_managed = false){
			stack = new Stack(state);
			if (stack->newMetatable("_lutok2")){
				//create local copy of State object in Lua State
				State ** _state = static_cast<State**> (stack->newUserData(sizeof(State*)));
				(*_state) = this;
					//prepare metatable for State object
					stack->newTable();
					if (lua_managed) {
						stack->setField<lua_CFunction>("__gc", free_current_state);
					}
						
					stack->setMetatable();
				stack->setField("State");
			}
			stack->pop();
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
			if (luaL_loadfile(state, fileName.c_str()) != 0){
				throw std::runtime_error("Can't open file: "+ fileName);
			}
		}

		void loadString(const std::string & chunk){
			if (luaL_loadstring(state, chunk.c_str()) != 0){
				throw std::runtime_error("Can't load a chunk");
			}
		}

		/*
			Libraries
		*/

		void registerLib(const module & members){
			assert(stack->is<LUA_TTABLE>());

			for (std::unordered_map< std::string, cxx_function >::const_iterator
				iter = members.begin(); iter != members.end(); iter++) {
					stack->push<std::string>((*iter).first);
					stack->push<cxx_function>((*iter).second);
					stack->setTable(-3);
			}
		}

		void registerLib(const module & members, const std::string & name, const int nup=0){
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
			vsprintf (buffer, fmt, args);
			luaL_error(state, "%s", buffer);
			va_end (args);
		}
		
		/*
			Class interfaces
		*/

		template<class C> void registerInterface(const std::string & name){
			BaseObject * interface = new C(this);
			interfaces[name] = interface;
			interface->getConstructor();
		}

		void registerInterface(const std::string & name, BaseObject * interface){
			interfaces[name] = interface;
			interface->getConstructor();
		}
		
		template<class C> C * getInterface(const std::string & name){
			C * iface = dynamic_cast<C*>(interfaces[name]);
			assert(iface);
			return iface;
		}
	};
};

#endif