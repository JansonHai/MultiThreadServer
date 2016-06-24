#ifndef SRC_HEADER_LUA_HANDLE_H_
#define SRC_HEADER_LUA_HANDLE_H_

#include "ByteArray.h"

struct lua_ctx
{
	int session;
	int proto;
	ReadByteArray * readByteArray;
	class fl_connection * conn;
};


int fl_run_lua_handle(struct lua_State * Lua, struct lua_ctx * ctx);

#endif /* SRC_HEADER_LUA_HANDLE_H_ */
