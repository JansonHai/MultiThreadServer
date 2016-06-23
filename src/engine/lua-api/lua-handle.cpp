#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include "lua-handle.h"

static int c_cont (lua_State *L, int status, lua_KContext ctx)
{
	return 0;
}


void fl_run_lua_handle(lua_State * Lua, struct lua_ctx * ctx)
{
	/* 使用 lua_pcallk，而不是lua_pcall */
	lua_getglobal(Lua, "fl_handle_main");
	lua_pushlightuserdata(Lua,(void *)ctx);
	lua_pushinteger(Lua, ctx->proto);
	lua_pushlightuserdata(Lua, (void *)ctx->readByteArray);
	int ret = lua_pcallk(Lua, 3, 0, 0, 0, c_cont);
	if(ret) {
		fprintf(stderr, "Lua Error: %s\n", lua_tostring(Lua, -1));
		lua_pop(Lua, 1);
	}
}
