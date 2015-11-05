require 'lfs'

local mkdir, chdir

if lfs then
	mkdir, chdir = lfs.mkdir, lfs.chdir
else
	mkdir = function(name) os.execute('mkdir '..name) end
	chdir = function(name) os.execute('cd '..name) end
end

local templates = {
	gitIgnore = [[
*.swp
*.sdf
*.suo
*.opensdf
*.user
*.so
*.so.*
build/*

]],
	cmakeMain = [[
cmake_minimum_required (VERSION 3.1.0)
project ({{project_name}})

# Include Lua 5.1

include(FindLua51)
include_directories(SYSTEM ${LUA_INCLUDE_DIR})

include_directories(dependencies/lutok2/include)

add_subdirectory(src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set_target_properties({{project_name}} PROPERTIES PREFIX "")

]],
	cmakeSrc = [[
set({{project_name}}_src
	main.cpp
)

set({{project_name}}_h
	common.hpp
)

set({{project_name}}_sources
	${{{project_name}}_src} ${{{project_name}}_h}
)

include_directories(./)

source_group("Header files" FILES ${{{project_name}}_h})

set({{project_name}}_version_script {{project_name}}.version)

IF(UNIX)
    IF(CMAKE_COMPILER_IS_GNUCC)
         SET(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY}hidden -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${{{project_name}}_version_script}")
    ENDIF(CMAKE_COMPILER_IS_GNUCC)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-declarations")
ENDIF(UNIX)

add_library({{project_name}} SHARED
	${{{project_name}}_sources}
)

set_property(TARGET {{project_name}} PROPERTY CXX_STANDARD 11)
set_property(TARGET {{project_name}} PROPERTY CXX_STANDARD_REQUIRED ON)

set_target_properties({{project_name}}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/bin"
)

configure_file({{lua_interface_name}}.lua {{lua_interface_name}}.lua COPYONLY)

target_link_libraries ({{project_name}}
	${LUA_LIBRARY}
)

]],
	versionFile = [[
{{project_name}} {
	global: luaopen_{{project_name}};
	local: *;
};

]],
	license = [[
	{{project_name}} - Lua binding for {{name}} library

	Copyright {{year}} {{author}}
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]],
	commonH = [[
/*
{{license}}
*/

#include <lutok2/lutok2.hpp>
using namespace lutok2;

#if (BUILDING_{{PROJECT_NAME}} || {{project_name}}_EXPORTS) && HAVE_VISIBILITY
#define {{PROJECT_NAME}}_DLL_EXPORTED __attribute__((visibility("default")))
#elif (BUILDING_{{PROJECT_NAME}} || {{project_name}}_EXPORTS) && defined _MSC_VER
#define {{PROJECT_NAME}}_DLL_EXPORTED __declspec(dllexport)
#elif defined _MSC_VER
#define {{PROJECT_NAME}}_DLL_EXPORTED __declspec(dllimport)
#else
#define {{PROJECT_NAME}}_DLL_EXPORTED
#endif

]],
	mainCpp = [[
/*
{{license}}
*/

#include "common.hpp"

namespace {{project_name}} {
	int init(State & state){
		return 0;
	}
};

extern "C" {{PROJECT_NAME}}_DLL_EXPORTED int luaopen_{{project_name}}(lua_State * L){
	State * state = new State(L);
	Stack * stack = state->stack;
	Module {{project_name}}_module;

	stack->newTable();
	
	{{project_name}}_module["init"] = {{project_name}}::init;

	state->registerLib({{project_name}}_module);
	return 1;
}

]],
	interfaceLua = [==[
--[[
	{{license}}
--]]

local {{name}} = require '{{project_name}}'
local M = {}

return M
]==]
}

local function applyTemplate(template, vars)
	local vars = vars or {}
	return (template:gsub("{{([_%w%d]+)}}", vars))
end

local function putFile(name, content)
	local f = assert(io.open(name, "w"))
	f:write(content)
	f:close()
end

local function git(cmdLine)
	local cmdLine = cmdLine or ''
	local f = io.popen('git '..cmdLine)
	local output = f:read('*a')
	f:close()
	return output
end

local function prepareGit(name)
    git('init .')
    git('submodule add https://github.com/soulik/lutok2.git dependencies/lutok2')
end

local function generateFiles(name)
	local vars = {
		project_name = 'lua'..name,
		PROJECT_NAME = ('lua'..name):upper(),
		interface_name = name,
		lua_interface_name = name,
		name = name,
		year = tostring(os.date('%Y')),
		author = git('config user.name'),
	}
	vars.license = applyTemplate(templates.license, vars)
	putFile('.gitignore', applyTemplate(templates.gitIgnore))
	putFile('CMakeLists.txt', applyTemplate(templates.cmakeMain, vars))
	putFile('src/CMakeLists.txt', applyTemplate(templates.cmakeSrc, vars))
	putFile('src/'..vars.project_name..'.version', applyTemplate(templates.versionFile, vars))
	putFile('src/'..vars.lua_interface_name..'.lua', applyTemplate(templates.interfaceLua, vars))
	putFile('src/common.hpp', applyTemplate(templates.commonH, vars))
	putFile('src/main.cpp', applyTemplate(templates.mainCpp, vars))
end

local function prepareProject(name)
	local baseDir = './'..name
	mkdir(baseDir)
	chdir(baseDir)
	prepareGit(name)
	mkdir('build')
	mkdir('dependencies')
	mkdir('doc')
	mkdir('src')
	mkdir('tests')
	generateFiles(name)
	git('add .')
	git([[commit -m "Initial commit"]])
end

if #arg>0 then
	prepareProject(tostring(arg[1]))
else
	print([[
Prepares a project directory for custom Lua 5.1 binding with Lutok2

Usage:]])
	print(arg[0], '<project_name>')
end
