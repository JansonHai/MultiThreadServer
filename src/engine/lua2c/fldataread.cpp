#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include <string>
#include <stdint.h>
#include "ByteArray.h"

static int read_int8(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	int8_t num = 0;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadInt8();
	}
	lua_pushinteger(L, num);
	return 1;
}

static int read_int16(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	int16_t num = 0;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadInt16();
	}
	lua_pushinteger(L, num);
	return 1;
}

static int read_int32(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	int32_t num = 0;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadInt32();
	}
	lua_pushinteger(L, num);
	return 1;
}

static int read_int64(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	int64_t num = 0LL;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadInt64();
	}
	lua_pushinteger(L, num);
	return 1;
}

static int read_float(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	float num = 0;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadFloat();
	}
	lua_pushnumber(L, num);
	return 1;
}

static int read_double(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	double num = 0;
	if (NULL != readByteArray)
	{
		num = readByteArray->ReadDouble();
	}
	lua_pushnumber(L, num);
	return 1;
}

static int read_string(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	std::string str("");
	if (NULL != readByteArray)
	{
		str = readByteArray->ReadString();
	}
	lua_pushlstring(L, str.c_str(), str.size());
	return 1;
}

static int reset_data(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	if (NULL != readByteArray)
	{
		readByteArray->ResetReadPos();
	}
	return 0;
}

static int release_data(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	if (NULL != readByteArray)
	{
		readByteArray->ReleaseBuffer();
	}
	return 0;
}

static int destory_data(struct lua_State * L)
{
	ReadByteArray * readByteArray = (ReadByteArray *)lua_touserdata(L, -1);
	if (NULL != readByteArray)
	{
		delete readByteArray;
	}
	return 0;
}

static struct luaL_Reg libs[] = {
		{"read_int8", read_int8},
		{"read_int16", read_int16},
		{"read_int32", read_int32},
		{"read_int64", read_int64},
		{"read_float", read_float},
		{"read_double", read_double},
		{"read_string", read_string},
		{"reset_data", reset_data},
		{"release_data", release_data},
		{"destory_data", destory_data},
		{NULL, NULL}
};

extern "C" int luaopen_fldataread(struct lua_State * L)
{
	luaL_newlib(L, libs);
	return 1;
}
