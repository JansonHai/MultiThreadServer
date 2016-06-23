#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <lua.hpp>
#include "envirment.h"
#include "ByteArray.h"
#include "MsgQueue.h"
#include "gamelogic.h"
#include "lua-handle.h"


static void * s_work_thread(void * arg);
//static void * s_lua_thread(void * arg);
static int s_run_state = 0;

static class MsgQueue<struct fl_gamelogic_ctx *> s_work_msg_queue;

#define MAX_CTX_LENGTH 102400
#define WORK_THREAD_NUM 4
static struct fl_gamelogic_ctx * s_ctxs;
static int s_ctx_pos = 0;
static pthread_mutex_t s_dispatch_message_lock;
static pthread_mutex_t s_work_thread_lock[WORK_THREAD_NUM];
static pthread_cond_t s_work_cond_lock[WORK_THREAD_NUM];
static bool s_work_thread_busy[WORK_THREAD_NUM];
static int s_work_id[WORK_THREAD_NUM];
static bool s_work_script_reload_flag[WORK_THREAD_NUM];

//static lua_State * Lua;

void fl_start_gamelogic()
{
	pthread_mutex_init(&s_dispatch_message_lock, NULL);
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		pthread_mutex_init(&s_work_thread_lock[i], NULL);
		pthread_cond_init(&s_work_cond_lock[i], NULL);
		s_work_thread_busy[i] = false;
		s_work_script_reload_flag[i] = false;
	}

	s_ctxs = (struct fl_gamelogic_ctx *)malloc(MAX_CTX_LENGTH * sizeof(struct fl_gamelogic_ctx));

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	s_run_state = 1;

	//create work thread
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		s_work_id[i] = i;
		pthread_create(&tid, &attr, s_work_thread, (void*)(&s_work_id[i]));
	}

}

void fl_stop_gamelogic()
{
	s_run_state = 0;
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		pthread_cond_signal(&s_work_cond_lock[i]);
	}
	struct fl_gamelogic_ctx * ctx;
	while (s_work_msg_queue.pop_message(ctx))
	{
		fl_free_message_data(ctx->message);
	}
	free(s_ctxs);
	pthread_mutex_destroy(&s_dispatch_message_lock);
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		pthread_mutex_destroy(&s_work_thread_lock[i]);
		pthread_cond_destroy(&s_work_cond_lock[i]);
	}

}

void fl_dispatch_message(class fl_connection * conn, struct fl_message_data * message)
{
	pthread_mutex_lock(&s_dispatch_message_lock);
	if (s_work_msg_queue.Size() < MAX_CTX_LENGTH)
	{
		struct fl_gamelogic_ctx * ctx = &s_ctxs[s_ctx_pos++];
		if (s_ctx_pos >= MAX_CTX_LENGTH) s_ctx_pos = 0;
		ctx->conn = conn;
		ctx->message = message;
		s_work_msg_queue.push_message(ctx);
	}
	else
	{
		fl_free_message_data(message);
	}
	pthread_mutex_unlock(&s_dispatch_message_lock);

	//wake up work thread
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		if (false == s_work_thread_busy[i])
		{
			pthread_mutex_lock(&s_work_thread_lock[i]);
			s_work_thread_busy[i] = true;
			pthread_cond_signal(&s_work_cond_lock[i]);
			pthread_mutex_unlock(&s_work_thread_lock[i]);
			break;
		}
	}
}


static void * s_work_thread(void * arg)
{
	int work_id = *((int*)arg);
	bool result = false;
	struct fl_gamelogic_ctx * ctx = NULL;
	struct fl_message_data * message = NULL;
	class fl_connection * conn = NULL;
	class ReadByteArray * readByteArray= NULL;
	int proto = 0;
	struct lua_ctx luactx;

	lua_State * Lua = luaL_newstate();
	luaL_openlibs(Lua);
	int status = luaL_dofile(Lua, fl_getenv("lua_main"));
	if (status != LUA_OK)
	{
		fl_log(2,"Can not load lua main file %s\n",fl_getenv("lua_main"));
	}

	while (true)
	{
		if (0 == s_run_state) break;
		if (true == s_work_script_reload_flag[work_id])
		{
			s_work_script_reload_flag[work_id] = false;
			lua_State * tmpLua = luaL_newstate();
			luaL_openlibs(tmpLua);
			int status = luaL_loadfile(tmpLua, fl_getenv("lua_main"));
			if (status != LUA_OK)
			{
				fl_log(2,"Can not reload lua main file %s\n", fl_getenv("lua_main"));
				lua_close(tmpLua);
			}
			else
			{
				lua_close(Lua);
				Lua = tmpLua;
			}
		}
		message = NULL;
		result = s_work_msg_queue.pop_message(ctx);
		if (false == result || NULL == ctx)
		{
			pthread_mutex_lock(&s_work_thread_lock[work_id]);
			s_work_thread_busy[work_id] = false;
			pthread_cond_wait(&s_work_cond_lock[work_id], &s_work_thread_lock[work_id]);
			pthread_mutex_unlock(&s_work_thread_lock[work_id]);
			continue;
		}
		s_work_thread_busy[work_id] = true;
		conn = ctx->conn;
		message = ctx->message;
		if (conn->GetSession() != message->session)
		{
			fl_free_message_data(message);
			message = NULL;
			continue;
		}

		//handle
		readByteArray = new ReadByteArray();
		if (NULL == readByteArray)
		{
			fl_free_message_data(message);
			continue;
		}
		readByteArray->SetReadContent(message->data, message->length);
		fl_free_message_data(message);
		proto = readByteArray->ReadInt32();
		readByteArray->ResetReadPos();
		if (0 != proto)
		{
			luactx.session = conn->GetSession();
			luactx.proto = proto;
			luactx.conn = ctx->conn;
			luactx.readByteArray = readByteArray;
			fl_run_lua_handle(Lua, &luactx);
		}
		readByteArray = NULL;
		message = NULL;
	}

	lua_close(Lua);
	pthread_exit(0);
}
