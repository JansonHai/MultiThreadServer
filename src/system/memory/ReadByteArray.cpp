#include <stdlib.h>
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
	char a[2];
};

//对于0x12345678
//大端：0x12 0x34 0x56 0x78
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

const char * ReadByteArray::GetBuffer()
{
	return m_buffer->buffer;
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
		return (char)0xff;
	}
	return m_buffer->buffer[m_cur_pos++];
}

int16_t ReadByteArray::ReadInt16()
{
	if (m_cur_pos + 2 > m_size)
	{
		return (int16_t)0xffff;
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
		return (int32_t)0xffffffff;
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
		return (int64_t)0xffffffffffffffffLL;
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
		return (unsigned char)0xff;
	}
	return (unsigned char)m_buffer->buffer[m_cur_pos++];
}

uint16_t ReadByteArray::ReadUInt16()
{
	if (m_cur_pos + 2 > m_size)
	{
		return (uint16_t)0xffff;
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
		return (uint32_t)0xffffffff;
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
		return (uint64_t)0xffffffffffffffffLL;
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
		return (float)0xffffffff;
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
		return (double)0xffffffffffffffffLL;
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

const char * ReadByteArray::ReadString()
{
	int32_t len = ReadInt32();
	if (len != 0xffffffff)
	{
		char * buf = &m_buffer->buffer[m_cur_pos];
		if (m_cur_pos + len > m_size)
		{
			return NULL;
		}
		m_cur_pos += len;
		return buf;
	}
	else
	{
		return NULL;
	}
	return NULL;
}
