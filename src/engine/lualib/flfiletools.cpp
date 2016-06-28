#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <lua.hpp>
#include <unistd.h>
#include "filetools.h"

static int pwd(struct lua_State * L)
{
	std::string path = fl_pwd();
	lua_pushlstring(L,path.c_str(),path.size());
	return 1;
}

static int cd(struct lua_State * L)
{
	const char * path = luaL_tolstring(L,-1, NULL);
	if (NULL != path)
	{
		fl_cd(path);
	}
	return 0;
}

static int getExt(struct lua_State * L)
{
	std::string ext("");
	const char * file = luaL_tolstring(L,-1, NULL);
	ext = fl_getext(file);
	lua_pushlstring(L,ext.c_str(),ext.size());
	return 1;
}

static int getDirs(struct lua_State * L)
{
	std::vector<std::string> list;
	const char * path = luaL_tolstring(L,-1, NULL);
	fl_getdirs(path, list);
	lua_newtable(L);
	int len = (int)list.size();
	for (int i=0;i<len;++i)
	{
		lua_pushnumber(L, i+1);    //key
		lua_pushlstring(L, list[i].c_str(), list[i].size());  //value
		lua_settable(L, -3);       //push key,value
	}
	return 1;
}

static int getFiles(struct lua_State * L)
{
	std::vector<std::string> list;
	const char * path = luaL_tolstring(L,-1, NULL);
	fl_getfiles(path, list);
	lua_newtable(L);
	int len = (int)list.size();
	for (int i=0;i<len;++i)
	{
		lua_pushnumber(L, i+1);    //key
		lua_pushlstring(L, list[i].c_str(), list[i].size());  //value
		lua_settable(L, -3);       //push key,value
	}
	return 1;
}

static int getAll(struct lua_State * L)
{
	std::vector<std::string> list;
	const char * path = luaL_tolstring(L,-1, NULL);
	fl_getdirall(path, list);
	lua_newtable(L);
	int len = (int)list.size();
	for (int i=0;i<len;++i)
	{
		lua_pushnumber(L, i+1);    //key
		lua_pushlstring(L, list[i].c_str(), list[i].size());  //value
		lua_settable(L, -3);       //push key,value
	}
	return 1;
}

static int isFile(struct lua_State * L)
{
	const char * file = luaL_tolstring(L,-1, NULL);
	bool ret = fl_isFile(file);
	lua_pushboolean(L, ret);
	return 1;
}

static int isFileExist(struct lua_State * L)
{
	const char * file = luaL_tolstring(L,-1, NULL);
	bool ret = fl_isFileExist(file);
	lua_pushboolean(L, ret);
	return 1;
}

static int isDir(struct lua_State * L)
{
	const char * file = luaL_tolstring(L,-1, NULL);
	bool ret = fl_isDir(file);
	lua_pushboolean(L, ret);
	return 1;
}

static struct luaL_Reg libs[] = {
	{"pwd", pwd},
	{"cd", cd},
	{"getExt", getExt},
	{"getDirs", getDirs},
	{"getFiles", getFiles},
	{"getAll", getAll},
	{"isFile", isFile},
	{"isFileExist", isFileExist},
	{"isDir", isDir},
	{NULL, NULL}
};

extern "C" int luaopen_flfiletools(struct lua_State * L)
{
	luaL_newlib(L, libs);
	return 1;
}
