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


WriteByteArray::WriteByteArray()
{
	m_buffer = NULL;
	m_size = 0;
}

WriteByteArray::~WriteByteArray()
{
	if (NULL != m_buffer)
	{
		ReleaseBuffer();
	}
}

void WriteByteArray::InitBuffer(int size)
{
	m_buffer = fl_malloc(size);
	m_size = 0;
}

const char * WriteByteArray::GetBuffer()
{
	if (NULL != m_buffer)
	{
		m_buffer->buffer;
	}
	return NULL;
}

int WriteByteArray::GetArraySize()
{
	return m_size;
}

void WriteByteArray::ReleaseBuffer()
{
	fl_free(m_buffer);
	m_buffer = NULL;
	m_size = 0;
}

void WriteByteArray::ResetWrite()
{
	m_size = 0;
}

void WriteByteArray::WriteSeek(int pos)
{
	if (pos <= m_buffer->size)
	{
		m_size = pos;
	}
}

bool WriteByteArray::WriteInt8(char i8)
{
	if (m_size + 1 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	m_buffer->buffer[m_size++] = i8;
	return true;
}

bool WriteByteArray::WriteInt16(int16_t i16)
{
	if (m_size + 2 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _Int16 t;
	t.n = i16;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteInt32(int32_t i32)
{
	if (m_size + 4 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _Int32 t;
	t.n = i32;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteInt64(int64_t i64)
{
	if (m_size + 8 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _Int64 t;
	t.n = i64;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[7];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[7];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteUInt8(unsigned char ui8)
{
	if (m_size + 1 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	m_buffer->buffer[m_size++] = ui8;
	return true;
}

bool WriteByteArray::WriteUInt16(uint16_t ui16)
{
	if (m_size + 2 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _UInt16 t;
	t.n = ui16;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteUInt32(uint32_t ui32)
{
	if (m_size + 4 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _UInt32 t;
	t.n = ui32;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteUInt64(uint64_t ui64)
{
	if (m_size + 8 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _UInt64 t;
	t.n = ui64;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[7];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[7];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteFloat(float f)
{
	if (m_size + 4 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _Float t;
	t.n = f;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteDouble(double d)
{
	if (m_size + 8 > m_buffer->size)
	{
		struct fl_buffer * tmp = fl_realloc(m_buffer,m_buffer->size << 1);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", m_buffer->size << 1);
			return false;
		}
		m_buffer = tmp;
	}
	union _Double t;
	t.n = d;
	if (s_is_big_endian())
	{
		m_buffer->buffer[m_size++] = t.byte[0];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[7];
	}
	else
	{
		m_buffer->buffer[m_size++] = t.byte[7];
		m_buffer->buffer[m_size++] = t.byte[6];
		m_buffer->buffer[m_size++] = t.byte[5];
		m_buffer->buffer[m_size++] = t.byte[4];
		m_buffer->buffer[m_size++] = t.byte[3];
		m_buffer->buffer[m_size++] = t.byte[2];
		m_buffer->buffer[m_size++] = t.byte[1];
		m_buffer->buffer[m_size++] = t.byte[0];
	}
	return true;
}

bool WriteByteArray::WriteString(const char * str)
{
	int len = strlen(str);
	if (m_size + len + 4 > m_buffer->size)
	{
		int size = m_buffer->size << 1;
		while (m_size + len + 4 > size) size <<= 1;
		struct fl_buffer * tmp = fl_realloc(m_buffer, size);
		if (NULL == tmp)
		{
			fl_log(2, "ByteArray try to relloc %d byte failed\n", size);
			return false;
		}
		m_buffer = tmp;
	}
	WriteInt32(len);
	if (0 != len)
	{
		memcpy(m_buffer->buffer, str, len);
		m_size += len;
	}
	return true;
}
