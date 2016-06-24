#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include <string>
#include <stdint.h>
#include "ByteArray.h"

static int write_int8(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	int8_t num = (int8_t)luaL_checkinteger(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteInt8(num);
	}
	return 0;
}

static int write_int16(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	int16_t num = (int16_t)luaL_checkinteger(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteInt16(num);
	}
	return 0;
}

static int write_int32(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	int32_t num = (int32_t)luaL_checkinteger(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteInt32(num);
	}
	return 0;
}

static int write_int64(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	int64_t num = (int64_t)luaL_checkinteger(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteInt64(num);
	}
	return 0;
}

static int write_float(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	float num = (float)luaL_checknumber(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteFloat(num);
	}
	return 0;
}

static int write_double(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	double num = (double)luaL_checknumber(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteDouble(num);
	}
	return 0;
}

static int write_string(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -2);
	size_t len = 0;
	const char * str = luaL_checklstring(L, -1, &len);
	if (NULL != writeByteArray)
	{
		writeByteArray->WriteString(str, (int)len);
	}
	return 0;
}

static int new_data(struct lua_State * L)
{
	WriteByteArray * writeByteArray = new WriteByteArray();
	lua_pushlightuserdata(L, writeByteArray);
	return 1;
}

static int reset_data(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->ResetWrite();
	}
	return 0;
}

static int release_data(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -1);
	if (NULL != writeByteArray)
	{
		writeByteArray->ReleaseBuffer();
	}
	return 0;
}

static int destory_data(struct lua_State * L)
{
	WriteByteArray * writeByteArray = (WriteByteArray *)lua_touserdata(L, -1);
	if (NULL != writeByteArray)
	{
		delete writeByteArray;
	}
	return 0;
}

static struct luaL_Reg libs[] = {
		{"write_int8", write_int8},
		{"write_int16", write_int16},
		{"write_int32", write_int32},
		{"write_int64", write_int64},
		{"write_float", write_float},
		{"write_double", write_double},
		{"write_string", write_string},
		{"new_data", new_data},
		{"reset_data", reset_data},
		{"release_data", release_data},
		{"destory_data", destory_data},
		{NULL, NULL}
};

extern "C" int luaopen_fldatawrite(struct lua_State * L)
{
	luaL_newlib(L, libs);
	return 1;
}
