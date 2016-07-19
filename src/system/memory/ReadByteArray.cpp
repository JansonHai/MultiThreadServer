#include <stdlib.h>
#include <string>
#include <algorithm>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include "ByteArray.h"
#include "logger.h"
#include "buffer.h"

union _Int16
{
	char byte[2];
	int16_t n;
};

union _Int32
{
	char byte[4];
	int32_t n;
};

union _Int64
{
	char byte[8];
	int64_t n;
};

union _UInt16
{
	char byte[2];
	uint16_t n;
};

union _UInt32
{
	char byte[4];
	uint32_t n;
};

union _UInt64
{
	char byte[8];
	uint64_t n;
};

union _Float
{
	char byte[4];
	float n;
};

union _Double
{
	char byte[8];
	double n;
};

union utype
{
	unsigned short n;
	unsigned char a[2];
};

//对于0x12345678
//数组: 0    1    2    3
//大端:0x12 0x34 0x56 0x78
//小端:0x78 0x56 0x34 0x12
//网络:0x12 0x34 0x56 0x78   即网络字节序为大端序
static inline bool s_is_big_endian()
{
	union utype t;
	t.n = 0x0102;
	return 0x01 == t.a[0];
}

ReadByteArray::ReadByteArray()
{
	m_buffer = NULL;
	m_size = 0;
	m_cur_pos = 0;
}

ReadByteArray::~ReadByteArray()
{
	if (NULL != m_buffer)
	{
		ReleaseBuffer();
	}
}

bool ReadByteArray::SetReadContent(const char * str, int size)
{
	if (NULL ==  m_buffer)
	{
		m_buffer = fl_malloc(size);
		if (NULL == m_buffer)
		{
			return false;
		}
	}
	if (size <= m_buffer->size)
	{
		memcpy(m_buffer->buffer, str, size);
		m_size = size;
		m_cur_pos = 0;
		return true;
	}
	else
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer, size);
		if (NULL == tmp)
		{
			return false;
		}
		m_buffer = tmp;
		memcpy(m_buffer->buffer, str, size);
		m_size = size;
		m_cur_pos = 0;
		return true;
	}
	return false;
}

void ReadByteArray::ResetReadPos()
{
	m_cur_pos = 0;
}

void ReadByteArray::ReadSeek(int pos)
{
	if (pos <= m_size)
	{
		m_cur_pos = pos;
	}
}

char * ReadByteArray::GetBuffer() const
{
	if (NULL != m_buffer)
	{
		return m_buffer->buffer;
	}
	return NULL;
}

int ReadByteArray::GetArraySize()
{
	return m_size;
}

void ReadByteArray::ReleaseBuffer()
{
	if (NULL != m_buffer)
	{
		fl_free(m_buffer);
	}
	m_buffer = NULL;
	m_size = 0;
	m_cur_pos = 0;
}

char ReadByteArray::ReadInt8()
{
	if (m_cur_pos + 1 > m_size)
	{
		return (char)0x00;
	}
	return (char)m_buffer->buffer[m_cur_pos++];
}

int16_t ReadByteArray::ReadInt16()
{
	if (m_cur_pos + 2 > m_size)
	{
		return (int16_t)0x0000;
	}
	union _Int16 i16;
	if (s_is_big_endian())
	{
		i16.byte[0] = m_buffer->buffer[m_cur_pos++];
		i16.byte[1] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		i16.byte[1] = m_buffer->buffer[m_cur_pos++];
		i16.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return i16.n;
}

int32_t ReadByteArray::ReadInt32()
{
	if (m_cur_pos + 4 > m_size)
	{
		return (int32_t)0x00000000;
	}
	union _Int32 i32;
	if (s_is_big_endian())
	{
		i32.byte[0] = m_buffer->buffer[m_cur_pos++];
		i32.byte[1] = m_buffer->buffer[m_cur_pos++];
		i32.byte[2] = m_buffer->buffer[m_cur_pos++];
		i32.byte[3] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		i32.byte[3] = m_buffer->buffer[m_cur_pos++];
		i32.byte[2] = m_buffer->buffer[m_cur_pos++];
		i32.byte[1] = m_buffer->buffer[m_cur_pos++];
		i32.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return i32.n;
}

int64_t ReadByteArray::ReadInt64()
{
	if (m_cur_pos + 8 > m_size)
	{
		return (int64_t)0x0000000000000000LL;
	}
	union _Int64 i64;
	if (s_is_big_endian())
	{
		i64.byte[0] = m_buffer->buffer[m_cur_pos++];
		i64.byte[1] = m_buffer->buffer[m_cur_pos++];
		i64.byte[2] = m_buffer->buffer[m_cur_pos++];
		i64.byte[3] = m_buffer->buffer[m_cur_pos++];
		i64.byte[4] = m_buffer->buffer[m_cur_pos++];
		i64.byte[5] = m_buffer->buffer[m_cur_pos++];
		i64.byte[6] = m_buffer->buffer[m_cur_pos++];
		i64.byte[7] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		i64.byte[7] = m_buffer->buffer[m_cur_pos++];
		i64.byte[6] = m_buffer->buffer[m_cur_pos++];
		i64.byte[5] = m_buffer->buffer[m_cur_pos++];
		i64.byte[4] = m_buffer->buffer[m_cur_pos++];
		i64.byte[3] = m_buffer->buffer[m_cur_pos++];
		i64.byte[2] = m_buffer->buffer[m_cur_pos++];
		i64.byte[1] = m_buffer->buffer[m_cur_pos++];
		i64.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return i64.n;
}

unsigned char ReadByteArray::ReadUInt8()
{
	if (m_cur_pos + 1 > m_size)
	{
		return (unsigned char)0x00;
	}
	return (unsigned char)m_buffer->buffer[m_cur_pos++];
}

uint16_t ReadByteArray::ReadUInt16()
{
	if (m_cur_pos + 2 > m_size)
	{
		return (uint16_t)0x0000;
	}
	union _UInt16 ui16;
	if (s_is_big_endian())
	{
		ui16.byte[0] = m_buffer->buffer[m_cur_pos++];
		ui16.byte[1] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		ui16.byte[1] = m_buffer->buffer[m_cur_pos++];
		ui16.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return ui16.n;
}

uint32_t ReadByteArray::ReadUInt32()
{
	if (m_cur_pos + 4 > m_size)
	{
		return (uint32_t)0x00000000;
	}
	union _UInt32 ui32;
	if (s_is_big_endian())
	{
		ui32.byte[0] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[1] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[2] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[3] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		ui32.byte[3] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[2] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[1] = m_buffer->buffer[m_cur_pos++];
		ui32.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return ui32.n;
}

uint64_t ReadByteArray::ReadUInt64()
{
	if (m_cur_pos + 8 > m_size)
	{
		return (uint64_t)0x0000000000000000LL;
	}
	union _UInt64 ui64;
	if (s_is_big_endian())
	{
		ui64.byte[0] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[1] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[2] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[3] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[4] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[5] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[6] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[7] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		ui64.byte[7] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[6] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[5] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[4] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[3] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[2] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[1] = m_buffer->buffer[m_cur_pos++];
		ui64.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return ui64.n;
}

float ReadByteArray::ReadFloat()
{
	if (m_cur_pos + 4 > m_size)
	{
		return (float)0x00000000;
	}
	union _Float f;
	if (s_is_big_endian())
	{
		f.byte[0] = m_buffer->buffer[m_cur_pos++];
		f.byte[1] = m_buffer->buffer[m_cur_pos++];
		f.byte[2] = m_buffer->buffer[m_cur_pos++];
		f.byte[3] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		f.byte[3] = m_buffer->buffer[m_cur_pos++];
		f.byte[2] = m_buffer->buffer[m_cur_pos++];
		f.byte[1] = m_buffer->buffer[m_cur_pos++];
		f.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return f.n;
}

double ReadByteArray::ReadDouble()
{
	if (m_cur_pos + 8 > m_size)
	{
		return (double)0x0000000000000000LL;
	}
	union _Double d;
	if (s_is_big_endian())
	{
		d.byte[0] = m_buffer->buffer[m_cur_pos++];
		d.byte[1] = m_buffer->buffer[m_cur_pos++];
		d.byte[2] = m_buffer->buffer[m_cur_pos++];
		d.byte[3] = m_buffer->buffer[m_cur_pos++];
		d.byte[4] = m_buffer->buffer[m_cur_pos++];
		d.byte[5] = m_buffer->buffer[m_cur_pos++];
		d.byte[6] = m_buffer->buffer[m_cur_pos++];
		d.byte[7] = m_buffer->buffer[m_cur_pos++];
	}
	else
	{
		d.byte[7] = m_buffer->buffer[m_cur_pos++];
		d.byte[6] = m_buffer->buffer[m_cur_pos++];
		d.byte[5] = m_buffer->buffer[m_cur_pos++];
		d.byte[4] = m_buffer->buffer[m_cur_pos++];
		d.byte[3] = m_buffer->buffer[m_cur_pos++];
		d.byte[2] = m_buffer->buffer[m_cur_pos++];
		d.byte[1] = m_buffer->buffer[m_cur_pos++];
		d.byte[0] = m_buffer->buffer[m_cur_pos++];
	}
	return d.n;
}

static std::string ReverseStr(const char * str, int len)
{
	std::string ss(str, len);
	int size = len - 1;
	int half = len / 2;
	char ch;
	for (int i=0;i<half;++i)
	{
		ch = ss[i];
		ss[i] = ss[size - i];
		ss[size - i] = ch;
	}
	return ss;
}

std::string ReadByteArray::ReadString()
{
	int len = ReadInt32();
	if (len == (int)0x00000000 || m_cur_pos + len > m_size)
	{
		return std::string("");
	}
	const char * buf = &m_buffer->buffer[m_cur_pos];
	m_cur_pos += len;
	std::string ret(buf, len);
	if (!s_is_big_endian())
	{
		std::reverse(ret.begin(), ret.end());
	}
	return ret;
//	struct fl_buffer * tmp = fl_malloc(len + 1);
//	memcpy(tmp->buffer, buf, len);
//	tmp->buffer[len] = '\0';
//	std::string ret(tmp->buffer);
//	fl_free(tmp);
//	return ret;
}
