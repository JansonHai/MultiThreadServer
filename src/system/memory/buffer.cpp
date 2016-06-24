#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "buffer.h"
#include "envirment.h"

struct fl_buffer_list
{
	int num;
	int config_num;
	int size;
	struct fl_buffer * next;
};

static struct fl_buffer_list list_stack[9];
static void s_real_free(struct fl_buffer * buffer);

static pthread_mutex_t s_malloc_lock[9];

void fl_init_buffer()
{
	int i,j,n,size;
	struct fl_buffer * buf;
	fl_buffer_list * stack;

	char keyNames[9][128] = {"buffer_1k_num","buffer_2k_num","buffer_4k_num","buffer_8k_num","buffer_16k_num","buffer_32k_num",
			"buffer_64k_num","buffer_128k_num","buffer_256k_num"};

	int defVals[9] = {4096,4096,2048,1024,256,128,128,64,64};

	for (i=0;i<9;++i)
	{
		pthread_mutex_init(&s_malloc_lock[i], NULL);
		n = fl_getenv(keyNames[i],defVals[i]);
		size = 1 << (10 + i);
		stack = &list_stack[i];
		stack->config_num = n;
		stack->num = 0;
		stack->size = size;
		stack->next = NULL;
		if (0 != n)
		{
			for (j=0;j<n;++j)
			{
				buf = (struct fl_buffer *)malloc(sizeof(struct fl_buffer));
				if (NULL == buf)
				{
					continue;
				}
				buf->buffer = (char *)malloc(size * sizeof(char));
				if (NULL == buf->buffer)
				{
					free(buf);
					continue;
				}
				buf->size = size;
				buf->next = stack->next;
				stack->next = buf;
				++stack->num;
			}
		}
	}
}

void fl_destory_buffer()
{
	struct fl_buffer * buf;
	struct fl_buffer * next;
	for (int i=0;i<9;++i)
	{
		next = list_stack[i].next;
		while (NULL != next)
		{
			buf = next;
			next = next->next;
			s_real_free(buf);
		}
		list_stack[i].next = NULL;
		list_stack[i].num = 0;
		pthread_mutex_destroy(&s_malloc_lock[i]);
	}
}

void fl_gc_buffer()
{
	struct fl_buffer * buf;
	for (int i=0;i<9;++i)
	{
		pthread_mutex_lock(&s_malloc_lock[i]);
		while (list_stack[i].num > list_stack[i].config_num)
		{
			buf = list_stack[i].next;
			list_stack[i].next = buf->next;
			--list_stack[i].num;
			s_real_free(buf);
		}
		pthread_mutex_unlock(&s_malloc_lock[i]);
	}
}

struct fl_buffer * fl_malloc(int size)
{
	int fit_size = 0;
	struct fl_buffer * buf;
	for (int i=0;i<9;++i)
	{
		if (size <= list_stack[i].size)
		{
			pthread_mutex_lock(&s_malloc_lock[i]);
			if (0 == fit_size) fit_size = list_stack[i].size;
			if (list_stack[i].num > 0)
			{
				--list_stack[i].num;
				buf = list_stack[i].next;
				list_stack[i].next = buf->next;
				buf->next = NULL;
				pthread_mutex_unlock(&s_malloc_lock[i]);
				return buf;
			}
			pthread_mutex_unlock(&s_malloc_lock[i]);
		}
	}
	if (0 != fit_size)
	{
		buf = (struct fl_buffer *)malloc(sizeof(struct fl_buffer));
		if (NULL == buf)
		{
			return NULL;
		}
		buf->buffer = (char *)malloc(fit_size * sizeof(char));
		if (NULL == buf->buffer)
		{
			free(buf);
			return NULL;
		}
		buf->size = fit_size;
		buf->next = NULL;
		return buf;
	}
	else
	{
		buf = (struct fl_buffer *)malloc(sizeof(struct fl_buffer));
		if (NULL == buf)
		{
			return NULL;
		}
		buf->buffer = (char *)malloc(size * sizeof(char));
		if (NULL == buf->buffer)
		{
			free(buf);
			return NULL;
		}
		buf->size = size;
		buf->next = NULL;
		return buf;
	}
	return NULL;
}


struct fl_buffer * fl_realloc(struct fl_buffer * _prebuffer,int size)
{
	struct fl_buffer * buf = NULL;
	if (NULL == _prebuffer)
	{
		buf = fl_malloc(size);
		goto _return_buf;
	}
	if (size <= _prebuffer->size)
	{
		buf = NULL;
		goto _return_buf;
	}
	int fit_size = 0;
	for (int i=0;i<9;++i)
	{
		if (size <= list_stack[i].size)
		{
			if (0 == fit_size) fit_size = list_stack[i].size;
			pthread_mutex_lock(&s_malloc_lock[i]);
			if (list_stack[i].num > 0)
			{
				buf = list_stack[i].next;
				list_stack[i].next = buf->next;
				buf->next = NULL;
				--list_stack[i].num;
				pthread_mutex_unlock(&s_malloc_lock[i]);
				break;
			}
			pthread_mutex_unlock(&s_malloc_lock[i]);
		}
	}
	if (NULL != buf)
	{
		memcpy(buf->buffer, _prebuffer->buffer, _prebuffer->size);
		fl_free(_prebuffer);
		goto _return_buf;
	}
	if (0 != fit_size)
	{
		buf = (struct fl_buffer *)malloc(sizeof(struct fl_buffer));
		if (NULL == buf)
		{
			buf = NULL;
			goto _return_buf;
		}
		buf->buffer = (char *)malloc(fit_size * sizeof(char));
		if (NULL == buf->buffer)
		{
			free(buf);
			buf = NULL;
			goto _return_buf;
		}
		buf->size = fit_size;
		buf->next = NULL;
		memcpy(buf->buffer, _prebuffer->buffer, _prebuffer->size);
		fl_free(_prebuffer);
		goto _return_buf;
	}
	else
	{
		buf = (struct fl_buffer *)malloc(sizeof(struct fl_buffer));
		if (NULL == buf)
		{
			buf = NULL;
			goto _return_buf;
		}
		buf->buffer = (char *)malloc(size * sizeof(char));
		if (NULL == buf->buffer)
		{
			free(buf);
			buf = NULL;
			goto _return_buf;
		}
		buf->size = size;
		buf->next = NULL;
		memcpy(buf->buffer, _prebuffer->buffer, _prebuffer->size);
		fl_free(_prebuffer);
		goto _return_buf;
	}

	_return_buf:
	return buf;
}

void fl_free(struct fl_buffer * buffer)
{
	for (int i=0;i<9;++i)
	{
		if (buffer->size == list_stack[i].size)
		{
			pthread_mutex_lock(&s_malloc_lock[i]);
			buffer->next = list_stack[i].next;
			list_stack[i].next = buffer;
			++list_stack[i].num;
			pthread_mutex_unlock(&s_malloc_lock[i]);
			return;
		}
	}
	s_real_free(buffer);
}

void s_real_free(struct fl_buffer * buffer)
{
	if (NULL == buffer) return;
	if (NULL != buffer->buffer)
		free(buffer->buffer);
	free(buffer);
}
