#ifndef HEADER_BYTEARRAY_H_
#define HEADER_BYTEARRAY_H_

#include <stdio.h>
#include <sys/types.h>

class ByteArray
{
private:
	struct fl_buffer * m_buffer;
	int m_size = 0;

public:
	ByteArray();
	~ByteArray();
	bool WriteInt8(char i);
	bool WriteInt16(int16_t i16);
	bool WriteInt32(int32_t i32);
	bool WriteInt64(int64_t i64);
	bool WriteUInt8(unsigned char ui);
	bool WriteUInt16(uint16_t ui16);
	bool WriteUInt32(uint32_t ui32);
	bool WriteUInt64(uint64_t ui64);
	bool WriteFloat(float f);
	bool WriteDouble(double d);
	bool WriteString(const char * str);
	void InitBuffer(int size = 2048);
	const char * GetBuffer();
	int GetSize();
	void ReleaseBuffer();
};

#endif /* HEADER_BYTEARRAY_H_ */
