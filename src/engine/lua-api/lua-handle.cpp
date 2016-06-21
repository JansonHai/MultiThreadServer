#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include "lua-handle.h"


void fl_run_lua_handle(lua_State * Lua, struct lua_ctx * ctx)
{
	lua_getglobal(Lua,"fl_handle_main");
	lua_pushlightuserdata(Lua,(void *)ctx);
	lua_pushinteger(Lua, ctx->proto);
	lua_pushlightuserdata(Lua, (void *)ctx->readByteArray);
	lua_pcall(Lua, 3, 0, 0);
}
