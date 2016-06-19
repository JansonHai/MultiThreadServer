#ifndef SRC_ENGINE_LUA_API_LUA_API_H_
#define SRC_ENGINE_LUA_API_LUA_API_H_

#include "ByteArray.h"

struct lua_ctx
{
	int session;
	ReadByteArray * readByteArray;
	class fl_connection * conn;
};



#endif /* SRC_ENGINE_LUA_API_LUA_API_H_ */
