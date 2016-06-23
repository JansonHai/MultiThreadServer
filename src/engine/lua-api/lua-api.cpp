#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include "lua-api.h"
#include "ByteArray.h"

extern "C" int release_data(struct lua_State * L)
{
//	ReadByteArray * readByteArray = (ReadByteArray *)lua(Lua, (void *)ctx->readByteArray);
	return 0;
}

static struct luaL_Reg libs[] = {
		{"release_data", release_data},
		{NULL, NULL}
};

extern "C" int luaopen_fllib(struct lua_State * L)
{
	luaL_newlib(L, libs);
	return 1;
}
