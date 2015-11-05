Lutok2
======

C++/Lua binding helper for Lua 5.1 and LuaJIT 2.x+.

Dependencies
============
To use Lutok2 successfully you'll need:

* A standards-compliant C++11 complier
* Lua 5.1.x or LuaJIT 2.0.x+ include and library files

Usage
=====
First of all, you'll need to add lutok2 *include* directory into list of include directories.
To use it, add following include line and you're good to go.

```cpp
#include <lutok2/lutok2.hpp>
```

All Lutok2 functions and classes are stored in *lutok2* namespace so you might actually want to use following two lines instead:

```cpp
#include <lutok2/lutok2.hpp>
using namespace lutok2;
```

API
===
API of Lutok2 library is divided into two sections:
* State - functions that manage Lua state, object interface registration
* Stack - functions that manage Lua stack
 
State
-----
* __new State()__ - creates a new empty Lua state.
* __new State(lua_State * L)__ - create an interface to existing Lua state (mostly used with Lua libraries).
* __~State()__ - closes Lua state if it's been created with `new State()` constructor.
* __stack__ - pointer to Stack object.
* __openLibs()__ - loads all standard Lua libraries into current State.
* __loadFile(const std::string & fileName)__ - loads/compiles a file with Lua source and pushes compiled function into stack.
* __loadString(const std::string & fileName)__ - loads/compiles a string with Lua source and pushes compiled function into stack.
* __error(const char * fmt, ...)__ - invokes error with formated message in current Lua state.
* __registerLib(const Module & members)__ - registers library functions. All functions are bound into Lua table which needs to be at top of the stack.
* __registerLib(const Module & members, const std::string & name, const int nup=0)__ - registers library functions for specific library.
* __\<classname\>registerInterface(const std::string & name)__ - registers a C++ class interface and pushes constructor function into stack. You should always use consistent class naming to avoid naming collisions.
* __getInterface\<classname\>(const std::string & name)__ - returns C++ class interface based on class name.

Stack
-----
### Stack manipulation
* __getTop()__ - returns number of stack items (arguments).
* __setTop()__ - sets stack position.
* __upvalueIndex(const int index)__ - returns pseudo-index of upvalue at specific index.
* __pop(int n=1)__ - pops n items from stack.
* __insert(int index)__ - insert items to the top of the stack.
* __replace(int index)__ - moves top item to specific location on the stack overwriting existing item.
* __remove(int index)__ - removes item at specific location (shifts other elements to close the gap).
* __getGlobal(const std::string & name)__ - pushes global variable into top of the stack.
* __setGlobal(const std::string & name)__ - stores item at top of the stack into global environment.

### Table manipulation
* __newTable()__ - creates a new table and pushes it to the top of the stack.
* __newTable(const int acount, const int nacount)__ - creates and preallocates a new table and pushes it to the top of the stack (acount - number of elements in sequence; nacount - number of other elements).
* __getTable(const int index = -3)__ - retrieves element from table at specific location. It takes top item from stack and uses it a key.
* __setTable(const int index = -3)__ - stores item into table at specific location. It takes the first two items from the top of the stack in this order: key, value.
* __concat(const int count)__ - concatenates number of items at the top of the stack.
* __rawGet(const int index = -3)__ - similar to `getTable`, skips `__index` metamethod invokation.
* __rawGet(const int n, const int index = -3)__ - retrieves n-th element from a table at specific location. Skips `__index` metmethod invokation.
* __rawSet(const int index = -3)__ - similar to `setTable`, skips `__newindex` metamethod invokation.
* __rawSet(const int n, const int index = -3)__ - set n-th element in a table at specific location. Skips `__newindex` metmethod invokation.
* __getField(const std::string & key, const int index = -2)__ - retrieves a named field value from a table at specific location.
* __getField(const int key, const int index = -2)__ - retrieves an indexed field value from a table at specific location.
* __setField(const std::string & key, const int index = -2)__ - stores element at the top of the stack into table field at specific location.
* __setField\<bool\>(const std::string & key, bool value, const int index)__ - stores a boolean value into table field.
* __setField\<int\>(const std::string & key, int value, const int index)__ - stores an integer value into table field.
* __setField\<LUA_NUMBER\>(const std::string & key, LUA_NUMBER value, const int index)__ - stores a numeric value into table field (a number is usually represented by double data type).
* __setField\<const char *\>(const std::string & key, const char * value, const int index)__ - stores null-terminated string value into table field.
* __setField\<const std::string &\>(const std::string & key, const std::string & value, const int index)__ - stores null-terminated string value into table field.
* __setField\<lua_CFunction\>(const std::string & key, lua_CFunction value, const int index)__ - stores C function into table field.
* __setField\<Function\>(const std::string & key, Function value, const int index)__ - stores C++ function into table field (you may use lambda function).
* __setField\<cxx_function\>(const std::string & key, cxx_function value, const int index)__ - stores C++ function into table field.
* __setField\<void *\>(const std::string & key, void * value, const int index)__ - stores pointer value into table field (it's stored as a lightuser data value).
* __setFieldLString(const std::string & name, const std::string & value, size_t len, const int index=-1)__ - stores a string value with specific length into table field (a string doesn't have to be null-terminated).

### Pushing values into stack
* __push\<int\>(int value)__ - pushes integer into stack.
* __push\<LUA_NUMBER\>(LUA_NUMBER value)__ - pushes a number into stack (usually, a number uses double data type). 
* __push\<bool\>(bool value)__ - pushes boolean value into stack.
* __push\<const char *\>(const char * value)__ - pushes null-terminated `char*` string into stack.
* __push\<const std::string &\>(const std::string & value)__ - pushes null-terminated `std::string` string into stack.
* __push\<void *\>(void * value)__ - pushes a pointer into stack (it's used as a lightuser data). 
* __push\<lua_CFunction\>(lua_CFunction value)__ - pushes C function into stack.
* __push\<Function\>(Function value)__ - pushes a C++ function into stack. You may use lambda function in this case.
* __push\<Function\>(Function value, int n)__ - pushes a C++ function into stack with n upvalues. You may use lambda function in this case.
* __push\<cxx_function\>(cxx_function value)__ - pushes a C++ function into stack.
* __push\<cxx_function\>(cxx_function value, int n)__ - pushes a C++ function into stack with n upvalues.
* __pushClosure(lua_CFunction fn, int n)__ - pushes C function into stack with n upvalues.
* __pushLString(const std::string & value, size_t len)__ - pushes a string with specific length into stack (string doesn't have to be null-terminated).
* __pushLString(const std::string & value)__ - pushes a string into stack (string doesn't have to be null-terminated). A string lLength is obtained from std::string object.
* __pushVFString(const char * fmt, ...)__ - pushes a formated string into stack.
* __pushLiteral(const std::string value)__ - pushes a literal value into stack.
* __pushNil()__ - pushes a nil value into stack.
* __pushValue(const int index)__ - pushes a value from specific location to the top the stack.

### Getting values from stack
* __to\<bool\>(const int index)__ - gets a boolean value from stack.
* __to\<int\>(const int index)__ - gets an integer value from stack.
* __to\<LUA_NUMBER\>(const int index)__ - gets a numeric value from stack (mostly represented with double data type).
* __to\<const std::string\>(const int index)__ - gets a string value from stack.
* __to\<void *\>(const int index)__ - gets a lightuser data pointer from stack.
* __toLString(const int index = -1)__ - gets a string value from stack (the string is not null-terminated).

### Value manipulation
* __objLen(const int index = -1)__ - returns value length at specific location.
* __type(const int index = -1)__ - returns value type (use with Lua `LUA_Ttypename` constants).
* __is\<LUA_Ttypename\>(const int index = -1)__ - returns true if there's a value with specified data type at specific location.
* __typeName(const int index = -1)__ - returns string representation of value data type at specific location.
* __newUserData(size_t size)__ - allocates specific amount (bytes) for full userdata. Userdata value is pushed to the stop of the stack.
* __checkUserData(const int narg, const std::string& name)__ - checks and returns full userdata pointer with specific name. Otherwise throws an error and returns `nullptr`.
* __getUserData(const int narg, const std::string& name)__ - similar to `checkUserData` except it doesn't throw an error message.

### Meta tables
* __getMetatable(const int index = -1)__ - pushes a metatable of a value at specific location.
* __getMetatable(const std::string & name)__ - pushes a metatable with specific name into stack.
* __setMetatable(const int index = -2)__ - sets metatable for a value at specific location. Metatable is retrieved from the top of the stack.
* __newMetatable(const std::string & name)__ - pushes a new metatable with specific name into stack. 
* __getMetaField(const std::string & name, const int index = -1)__ - pushes specific metatable field from a value at specific location into stack.
 
### Functions
* __call(const int nargs, const int nresults)__ - calls a function at the top of the stack with `nargs` arguments and expects `nresults` values on return.
* __pcall(const int nargs, const int nresults, const int errFunction = 0)__ - similar to `call` except you can defined error function from specific location and in case of error message it throws a runtime exception which you can manage with `catch`.

### Lua registry
* __ref(const int index = LUA_REGISTRYINDEX)__ - stores a value at the top of the stack into registry and returns integer reference number which you can use to identify stored item.
* __unref(const int ref, const int index = LUA_REGISTRYINDEX)__ - removes a reference to a value specified with reference number.
* __regValue(const int n)__ - retrieves item from Lua registry with specified reference number.
 
Examples
========

Lutok2 usage in a library
---------------------------
```cpp
#include <lutok2/lutok2.hpp>
using namespace lutok2;

#if (BUILDING_MyLibrary || MyLibrary_EXPORTS) && HAVE_VISIBILITY
#define MyLibrary_DLL_EXPORTED __attribute__((visibility("default")))
#elif (BUILDING_MyLibrary || MyLibrary_EXPORTS) && defined _MSC_VER
#define MyLibrary_DLL_EXPORTED __declspec(dllexport)
#elif defined _MSC_VER
#define MyLibrary_DLL_EXPORTED __declspec(dllimport)
#else
#define MyLibrary_DLL_EXPORTED
#endif

namespace MyLibrary {
    class MyClass {
    public:
        std::string value;
        int method(int param1){
            return param1 + 1;
        };
    };
    
	class LuaMyClass : public Object<MyClass> {
	public:
	    // Define all properties and methods here
		explicit LuaMyClass(State * state) : Object<MyClass>(state){
			LUTOK_PROPERTY("value", &LuaMyClass::getValue, &LuaMyClass::setValue);
			LUTOK_PROPERTY("valueGetterOnly", &LuaMyClass::getValue, &LuaMyClass::nullMethod);
			LUTOK_PROPERTY("valueSetterOnly", &LuaMyClass::nullMethod, &LuaMyClass::setValue);
			LUTOK_METHOD("method", &LuaMyClass::method);
		}

		MyClass * constructor(State & state, bool & managed);
		void destructor(State & state, MyClass * object);

	    int method(State & state, MyClass * object);
	    int getValue(State & state, MyClass * object);
	    int setValue(State & state, MyClass * object);
	};

    int LuaMyClass::method(State & state, MyClass * object){
        Stack * stack = state.stack; //a shortcut
        if (stack->is<LUA_TNUMBER>(1)){
            int param1 = stack->to<int>(1);
            int result = object->method(param1);
            stack->push<int>(result);
            return 1;
        }else{
            state.error("Expects a number");
            return 0;
        }
    }
    
    // Initialize object
    MyClass * LuaMyClass::constructor(State & state, bool & managed){
        MyClass * object = new MyClass;
        object->value = "Unspecified value";
        return object;
    }
    
    // Don't forget to release alloced memory inside of object
    void LuaMyClass::destructor(State & state, MyClass * object){
        delete object;
    }
    
    int LuaMyClass::getValue(State & state, MyClass * object){
        Stack * stack = state.stack; //a shortcut
        stack->push<const std::string &>(object->value);
        return 1;
    }
    
    int LuaMyClass::setValue(State & state, MyClass * object){
        Stack * stack = state.stack; //a shortcut
        if (stack->is<LUA_TSTRING>(1)){
            const std::string inputValue = stack->to<const std::string>(1);
            object->value = inputValue;
        }
        return 0;
    }

	// Must be called with a table value on stack - because of setField
	void initLuaMyClass(State* state, Module& module){
	    Stack * stack = state->stack; //a shortcut
	    state->registerInterface<LuaMyClass>("MyLibrary_LuaMyClass");
	    // Register LuaMyClass constructor in current table interface
	    stack->setField("LuaMyClass")
	}

	int init(State & state){
		return 0;
	}
	
	int processObject(State & state){
        LuaMyClass * interfaceLuaMyClass = state.getInterface<LuaMyClass>("MyLibrary_LuaMyClass");
        // Retrieve MyClass object from Lua - in the 1st argument
        MyClass * object = interfaceLuaMyClass->get(1);
        if (object){
            MyClass * newObject = new MyClass;
            newObject->value = object->value;
            /*  !!! IMPORTANT !!!
                1. You need to set second argument (luaManaged value) to true to activate automatic object cleanup
                which will call destructor on GC. Otherwise you risk memory leaks!
                
                2. If you're pushing objects that are managed by other classes, set this parameter to false.
                GC will not call destructor when the Lua object is collected.
                You can think of this as a light object reference.
            */
            bool luaManaged = true;
            interfaceLuaMyClass->push(newObject, luaManaged);
            return 1;
        }else{
            state.error("Expects a LuaMyClass object");
            return 0;
        }
	}
};

extern "C" MyLibrary_DLL_EXPORTED int luaopen_MyLibrary(lua_State * L){
	State * state = new State(L);
	Stack * stack = state->stack;
	Module MyLibrary_module;

	stack->newTable();
	
	initLuaMyClass(state, MyLibrary_module);
	
	MyLibrary_module["init"] = MyLibrary::init;
	MyLibrary_module["processObject"] = MyLibrary::processObject;

	state->registerLib(MyLibrary_module);
	return 1;
}

```

Lutok2 usage in application
---------------------------
```cpp
#include "lutok2/lutok2.hpp"

using namespace lutok2;

int testFun(State & state){
	state.loadString(
		"print(\"Hello world from lambda function!\") \
						"
						);
	state.stack->call(0, 0);
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

class LTestObj : public Object < TestObj > {
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
		state.stack->call(0, 0);
		return 0;
	});
	state.stack->setGlobal("testing");

	state.registerInterface<LTestObj>("testObj");
	state.stack->setGlobal("testObj");

	try {
		state.loadFile("test/test.lua");
		state.stack->call(0, 0);
	}
	catch (std::exception & e){
		printf("Can't load test file: %s", e.what());
	}
	return 0;
}
```

Authors
=======
* Mário Kašuba <soulik42@gmail.com>

